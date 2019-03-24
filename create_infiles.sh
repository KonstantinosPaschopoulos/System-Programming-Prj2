#!/bin/bash
declare -a dir_names

# Checks the number of arguments
if [ "$#" -ne 4 ]; then
	echo Correct use: ./create_infiles.sh dir_name num_of_files num_of_dirs levels
	exit 1
fi

# Checks if the first argument exists
if [ -e "$1" ]; then
	# Checks if the first argument is a directory
	if ! [ -d "$1" ]; then
		echo $1 exists, but it is not a directory
		exit 1
	fi
else
	mkdir $1
fi

# Creating the directory names
for ((i=1; i <= $3; i++)); do
	# Deciding how long the name of each directory will be using shuf
	length=`shuf -i 1-8 -n 1`
	tmp=`head /dev/urandom | tr -dc A-Za-z0-9 | head -c $length`
	# Storing the random alphanumeric string in the array
	dir_names[$i]="$tmp";
done

# Printing the array
# for ((i=1; i <= $3; i++)); do
# 	echo ${dir_names[$i]}
# done
# echo ${dir_names[*]}

# Creating the directories
