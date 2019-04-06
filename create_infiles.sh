#!/bin/bash

# Usage: ./create_infiles.sh dir_name num_of_files num_of_dirs levels

declare -a dir_names
declare -a file_names

# Checks the number of arguments
if [ $# -ne 4 ]; then
	echo Correct use: ./create_infiles.sh dir_name num_of_files num_of_dirs levels
	exit 1
fi

# Check if the rest of the arguments are non negative numbers
if [ $2 -ge 0 ]; then
	echo -n
else
	echo $2 is not an acceptable number
	exit 1
fi
if [ $3 -ge 0 ]; then
	echo -n
else
	echo $3 is not an acceptable number
	exit 1
fi
if [ $4 -ge 0 ]; then
	echo -n
else
	echo $4 is not an acceptable number
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
dir_names[0]="$1";
for ((i=1; i <= $3; i++)); do
	tmp=`./randomString 1 8`
	# Storing the random alphanumeric string in an array
	dir_names[$i]="$tmp";
done

# Creating the num_of_dirs directories
echo Creating the directories:
if [ $4 -ge 1 ]; then
	i=1
	# The external while loop counts how many directories have been created
	while [ $i -le $3 ]; do
		path="$1"
		j=0
		# The internal loop counts how many levels have been created
		while [ $j -lt $4 ]; do
			if [ $(( j+i )) -gt $3 ]; then
				break
			fi
			path="${path}/${dir_names[i+j]}"
			mkdir -p "$path"
			echo "$path"
			j=$(( j+1 ))
		done
		i=$(( i+j ))
	done
fi

# Creating the file names
for ((i=0; i < $2; i++)); do
	tmp=`./randomString 1 8`
	# Storing the random alphanumeric string in an array
	file_names[$i]="$tmp";
done

echo
echo Creating the files:
y=0
# The external while loop checks how many files have been created
while [ $y -lt $2 ]; do
	if [ $3 -eq 0 ]; then
		# In the case that no directories have been created everything goes in the dir_name
		path="$1"
		touch "${path}/${file_names[y]}"
		# Choosing a random number between 1kb and 128kb
		tmpString=`./randomString 1024 131072`
		echo "$tmpString" > "${path}/${file_names[y]}"
		echo "${path}/${file_names[y]}"
		y=$(( y+1 ))
	else
		i=1
		path="$1"
		touch "${path}/${file_names[y]}"
		# Choosing a random number between 1kb and 128kb
		tmpString=`./randomString 1024 131072`
		echo "$tmpString" > "${path}/${file_names[y]}"
		echo "${path}/${file_names[y]}"
		y=$(( y+1 ))
		# Looping around the directories to achieve a round-robin distribution
		while [ $i -le $3 ]; do
			if [ $y -ge $2 ]; then
				break
			fi
			path="$1"
			j=0
			while [ $j -lt $4 ]; do
				if [ $(( j+i )) -gt $3 ]; then
					break
				fi
				if [ $y -ge $2 ]; then
					break
				fi
				path="${path}/${dir_names[i+j]}"
				touch "${path}/${file_names[y]}"
				# Choosing a random number between 1kb and 128kb
				tmpString=`./randomString 1024 131072`
				echo "$tmpString" > "${path}/${file_names[y]}"
				echo "${path}/${file_names[y]}"
				y=$(( y+1 ))
				j=$(( j+1 ))
			done
			i=$(( i+j ))
		done
	fi
done
