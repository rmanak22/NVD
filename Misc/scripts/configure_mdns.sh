#!/bin/bash
# mDNS Configuration Script for macOS
# This script forces the hostname to metallyze-server (advertised as metallyze-server.local)
# and optionally advertises an HTTP service (e.g., a Flask server) using mDNS.
#
# Note: Changing the hostname requires administrative privileges.

echo "=== mDNS Configuration for macOS ==="
echo "Forcing hostname to metallyze-server (advertised as metallyze-server.local)..."

# Set hostname values without underscores (hyphen used instead)
sudo scutil --set ComputerName "metallyze-server"
sudo scutil --set LocalHostName "metallyze-server"
sudo scutil --set HostName "metallyze-server.local"

echo "Hostname successfully set. Your mDNS name is now metallyze-server.local."

# Optionally advertise an HTTP service via mDNS
read -p "Do you want to advertise an HTTP service (e.g., a Flask server) via mDNS? (y/n): " ADVERTISE

if [[ "$ADVERTISE" =~ ^[Yy]$ ]]; then
    SERVICE_NAME="My Flask Server"
    SERVICE_TYPE="_http._tcp"
    DEFAULT_PORT=80

    read -p "Enter the port for the HTTP service (default ${DEFAULT_PORT}): " SERVICE_PORT
    if [ -z "$SERVICE_PORT" ]; then
      SERVICE_PORT=$DEFAULT_PORT
    fi

    echo "Advertising '$SERVICE_NAME' on metallyze-server.local (port ${SERVICE_PORT})..."
    # The dns-sd command will advertise the service until terminated.
    dns-sd -R "$SERVICE_NAME" "$SERVICE_TYPE" local $SERVICE_PORT &
    ADVERT_PID=$!
    echo "Service advertisement started with PID: $ADVERT_PID."
    echo "To stop the advertisement later, run: kill $ADVERT_PID"
else
    echo "Skipping service advertisement."
fi

echo "mDNS configuration complete."
