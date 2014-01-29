# Get time command
echo -n -e \\x31 > gnd-input
echo -n -e \\x21 > gnd-input

# Upload file command
# Will upload a file named test.txt to /home/logs
mkdir -p /home/logs
sudo chmod 777 /home/logs
echo -n -e \\x32\\x30\\x31\\x39\\x2f\\x68\\x6f\\x6d\\x65\\x2f\\x6c\\x6f\\x67\\x73\\x2f\\x74\\x65\\x73\\x74\\x2e\\x74\\x78\\x74\\x30\\x31\\x32\\x68\\x65\\x6c\\x6c\\x6f\\x20\\x73\\x70\\x61\\x63\\x65\\xa > gnd-input

echo -n -e \\x21 > gnd-input

# Get logs command
# params: subsystem, # of bytes to get
echo -n -e \\x33\\x30\\x31\\x35\\x30 > gnd-input
echo -n- e \\x21 > gnd-input

# TODO: Use same logic as command for reconstruction of files
# Look for a better IPC method than pipes
