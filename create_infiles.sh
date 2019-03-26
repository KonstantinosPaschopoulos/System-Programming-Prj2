#!/bin/bash
declare -a dir_names

# Checks the number of arguments
if [ $# -ne 4 ]; then
	echo Correct use: ./create_infiles.sh dir_name num_of_files num_of_dirs levels
	exit 1
fi

# Checks if the first argument exists
if [ -e "$1" ]; then
	# Checks if the first argument is a directory
	if ! [ -d "$1" ]; then
		echo "$1" exists, but it is not a directory
		exit 1
	fi
else
	mkdir -p "$1"
fi

# Creating the directory names
for ((i=0; i < $3; i++)); do
	# Deciding how long the name of each directory will be using shuf
	length=`shuf -i 1-8 -n 1`
	tmp=`head /dev/urandom | tr -dc A-Za-z0-9 | head -c $length`
	# Storing the random alphanumeric string in an array
	dir_names[$i]="$tmp";
done

# Creating the num_of_dirs directories
i=0
# The external while loop counts how many directories have been created
while [ $i -lt $3 ]; do
	path="$1/${dir_names[$i]}"
	mkdir -p "$path"
	j=1
	# The internal loop counts how many levels have been created
	while [ $j -lt $4 ]; do
		if [ $(( j+i )) -gt $3 ]; then
			exit 0
		fi
		path="${path}/${dir_names[i+j]}"
		mkdir -p "$path"
		echo "$path"
		j=$(( j+1 ))
	done
	i=$(( i+j ))
done
