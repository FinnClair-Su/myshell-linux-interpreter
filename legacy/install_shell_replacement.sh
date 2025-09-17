#!/bin/bash

# Shell Installation and Replacement Script
# Safely installs myshell as a system shell replacement

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Configuration
SHELL_NAME="myshell"
INSTALL_DIR="/usr/local/bin"
SHELLS_FILE="/etc/shells"
BACKUP_SUFFIX=".backup.$(date +%Y%m%d_%H%M%S)"

# Function to print colored output
print_status() {
    local status=$1
    local message=$2
    case $status in
        "INFO")  echo -e "${BLUE}[INFO]${NC} $message" ;;
        "WARN")  echo -e "${YELLOW}[WARN]${NC} $message" ;;
        "ERROR") echo -e "${RED}[ERROR]${NC} $message" ;;
        "SUCCESS") echo -e "${GREEN}[SUCCESS]${NC} $message" ;;
    esac
}

# Function to check if running as root
check_root() {
    if [ "$EUID" -eq 0 ]; then
        print_status "WARN" "Running as root. This will install system-wide."
        return 0
    else
        print_status "INFO" "Not running as root. Will attempt user installation."
        INSTALL_DIR="$HOME/bin"
        return 1
    fi
}

# Function to check prerequisites
check_prerequisites() {
    print_status "INFO" "Checking prerequisites..."
    
    # Check if myshell exists
    if [ ! -f "$SHELL_NAME" ]; then
        print_status "ERROR" "myshell executable not found. Please build it first."
        exit 1
    fi
    
    # Check if myshell is executable
    if [ ! -x "$SHELL_NAME" ]; then
        print_status "ERROR" "myshell is not executable."
        exit 1
    fi
    
    # Test basic functionality
    if ! echo "exit" | timeout 5 ./"$SHELL_NAME" > /dev/null 2>&1; then
        print_status "ERROR" "myshell basic functionality test failed."
        exit 1
    fi
    
    print_status "SUCCESS" "Prerequisites check passed."
}

# Function to create installation directory
create_install_dir() {
    if [ ! -d "$INSTALL_DIR" ]; then
        print_status "INFO" "Creating installation directory: $INSTALL_DIR"
        mkdir -p "$INSTALL_DIR"
    fi
    
    if [ ! -w "$INSTALL_DIR" ]; then
        print_status "ERROR" "Cannot write to installation directory: $INSTALL_DIR"
        exit 1
    fi
}

# Function to install shell
install_shell() {
    local target_path="$INSTALL_DIR/$SHELL_NAME"
    
    print_status "INFO" "Installing shell to $target_path"
    
    # Backup existing installation if it exists
    if [ -f "$target_path" ]; then
        print_status "WARN" "Existing installation found. Creating backup."
        cp "$target_path" "$target_path$BACKUP_SUFFIX"
    fi
    
    # Copy shell to installation directory
    cp "$SHELL_NAME" "$target_path"
    chmod 755 "$target_path"
    
    print_status "SUCCESS" "Shell installed to $target_path"
}

# Function to add shell to /etc/shells
add_to_shells_file() {
    local shell_path="$INSTALL_DIR/$SHELL_NAME"
    
    if [ ! -f "$SHELLS_FILE" ]; then
        print_status "WARN" "/etc/shells not found. Creating it."
        if [ "$EUID" -eq 0 ]; then
            touch "$SHELLS_FILE"
        else
            print_status "ERROR" "Cannot create /etc/shells without root privileges."
            return 1
        fi
    fi
    
    # Check if shell is already in /etc/shells
    if grep -q "^$shell_path$" "$SHELLS_FILE" 2>/dev/null; then
        print_status "INFO" "Shell already listed in $SHELLS_FILE"
        return 0
    fi
    
    # Add shell to /etc/shells
    print_status "INFO" "Adding shell to $SHELLS_FILE"
    
    if [ "$EUID" -eq 0 ]; then
        # Backup /etc/shells
        cp "$SHELLS_FILE" "$SHELLS_FILE$BACKUP_SUFFIX"
        echo "$shell_path" >> "$SHELLS_FILE"
        print_status "SUCCESS" "Shell added to $SHELLS_FILE"
    else
        print_status "WARN" "Cannot modify $SHELLS_FILE without root privileges."
        print_status "INFO" "Please run as root or manually add: $shell_path"
        return 1
    fi
}

# Function to test installation
test_installation() {
    local shell_path="$INSTALL_DIR/$SHELL_NAME"
    
    print_status "INFO" "Testing installation..."
    
    # Test basic execution
    if echo "exit" | timeout 5 "$shell_path" > /dev/null 2>&1; then
        print_status "SUCCESS" "Installation test passed."
    else
        print_status "ERROR" "Installation test failed."
        return 1
    fi
    
    # Test with some commands
    if echo -e "pwd\necho Installation test\nexit" | timeout 10 "$shell_path" > /dev/null 2>&1; then
        print_status "SUCCESS" "Command execution test passed."
    else
        print_status "WARN" "Command execution test failed."
    fi
}

# Function to show usage instructions
show_usage_instructions() {
    local shell_path="$INSTALL_DIR/$SHELL_NAME"
    
    print_status "INFO" "Installation completed successfully!"
    echo
    echo "Shell installed at: $shell_path"
    echo
    echo "To use myshell as your default shell:"
    echo "1. Verify the shell is working:"
    echo "   $shell_path"
    echo
    echo "2. Change your default shell:"
    echo "   chsh -s $shell_path"
    echo
    echo "3. Log out and log back in to use the new shell"
    echo
    echo "To revert to your previous shell:"
    echo "   chsh -s /bin/bash  # or your preferred shell"
    echo
    echo "Note: You can always start myshell manually by running:"
    echo "   $shell_path"
}

# Function to create uninstall script
create_uninstall_script() {
    local uninstall_script="uninstall_myshell.sh"
    local shell_path="$INSTALL_DIR/$SHELL_NAME"
    
    cat > "$uninstall_script" << EOF
#!/bin/bash
# MyShell Uninstall Script

echo "Uninstalling MyShell..."

# Remove from /etc/shells if present
if [ -f "$SHELLS_FILE" ] && grep -q "^$shell_path$" "$SHELLS_FILE"; then
    echo "Removing from $SHELLS_FILE..."
    if [ "\$EUID" -eq 0 ]; then
        sed -i '\|^$shell_path$|d' "$SHELLS_FILE"
        echo "Removed from $SHELLS_FILE"
    else
        echo "Warning: Cannot modify $SHELLS_FILE without root privileges."
        echo "Please manually remove: $shell_path"
    fi
fi

# Remove installed shell
if [ -f "$shell_path" ]; then
    echo "Removing $shell_path..."
    rm -f "$shell_path"
    echo "Shell removed."
else
    echo "Shell not found at $shell_path"
fi

# Restore backups if they exist
if [ -f "$shell_path$BACKUP_SUFFIX" ]; then
    echo "Backup found. Restore? (y/n)"
    read -r response
    if [ "\$response" = "y" ] || [ "\$response" = "Y" ]; then
        mv "$shell_path$BACKUP_SUFFIX" "$shell_path"
        echo "Backup restored."
    fi
fi

echo "Uninstall completed."
echo "Note: If myshell was your default shell, change it back with:"
echo "  chsh -s /bin/bash"
EOF

    chmod +x "$uninstall_script"
    print_status "INFO" "Created uninstall script: $uninstall_script"
}

# Function to run interactive installation
interactive_install() {
    echo "MyShell Installation Wizard"
    echo "==========================="
    echo
    
    echo "This will install myshell as a system shell replacement."
    echo "Installation directory: $INSTALL_DIR"
    echo
    
    read -p "Continue with installation? (y/n): " -r
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        print_status "INFO" "Installation cancelled."
        exit 0
    fi
    
    echo
    read -p "Add shell to /etc/shells? (y/n): " -r
    local add_to_shells=$REPLY
    
    echo
    read -p "Create uninstall script? (y/n): " -r
    local create_uninstall=$REPLY
    
    echo
    print_status "INFO" "Starting installation..."
    
    check_prerequisites
    create_install_dir
    install_shell
    
    if [[ $add_to_shells =~ ^[Yy]$ ]]; then
        add_to_shells_file
    fi
    
    test_installation
    
    if [[ $create_uninstall =~ ^[Yy]$ ]]; then
        create_uninstall_script
    fi
    
    show_usage_instructions
}

# Main function
main() {
    case "${1:-interactive}" in
        "install")
            check_root
            check_prerequisites
            create_install_dir
            install_shell
            add_to_shells_file
            test_installation
            create_uninstall_script
            show_usage_instructions
            ;;
        "test")
            check_prerequisites
            print_status "SUCCESS" "Shell is ready for installation."
            ;;
        "interactive"|"")
            check_root
            interactive_install
            ;;
        "help"|"-h"|"--help")
            echo "Usage: $0 [install|test|interactive|help]"
            echo
            echo "Commands:"
            echo "  install     - Automatic installation"
            echo "  test        - Test shell readiness"
            echo "  interactive - Interactive installation (default)"
            echo "  help        - Show this help"
            ;;
        *)
            print_status "ERROR" "Unknown command: $1"
            echo "Use '$0 help' for usage information."
            exit 1
            ;;
    esac
}

# Run main function with all arguments
main "$@"