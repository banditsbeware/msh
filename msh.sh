echo "building mav shell..."
rm ./msh
gcc msh.c -o msh -std=c99
echo "running mav shell..."
echo
./msh
