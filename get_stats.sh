#!/bin/bash

# Usage: cat log_file1 log_file2 ... log_filen | ./get_stats.sh

declare -a ids

flag=0
connected_ids=0
min_id=0
max_id=0
bytes_sent=0
bytes_read=0
files_sent=0
files_read=0
disconnected_ids=0

i=0
while read -r action result; do
  if [ "CLIENT_CONNECTED" == "$action"  ]; then
    connected_ids=$(( connected_ids+1 ))

    ids[$i]="$result"
    i=$(( i+1 ))

    if [ $flag -eq 0 ]; then
      flag=1
      max_id=$result
      min_id=$result
      continue
    fi
    if [ $result -gt $max_id ]; then
      max_id=$result
    fi
    if [ $result -lt $min_id ]; then
      min_id=$result
    fi

    continue
  fi

  if [ "BYTES_SENT" == "$action"  ]; then
    bytes_sent=$(( bytes_sent+$result ))
    continue
  fi

  if [ "BYTES_RECEIVED" == "$action"  ]; then
    bytes_read=$(( bytes_read+$result ))
    continue
  fi

  if [ "FILE_SENT" == "$action"  ]; then
    files_sent=$(( files_sent+1 ))
    continue
  fi

  if [ "FILE_RECEIVED" == "$action"  ]; then
    files_read=$(( files_read+1 ))
    continue
  fi

  if [ "CLIENT_DISCONNECTED" == "$action"  ]; then
    disconnected_ids=$(( disconnected_ids+1 ))
    continue
  fi
done

echo Number of connected ids: $connected_ids
echo Ids: "${ids[*]}"
echo Minimun id: $min_id
echo Maximum id: $max_id
echo Bytes sent: $bytes_sent
echo Bytes read: $bytes_read
echo Files sent: $files_sent
echo Files read: $files_read
echo Number of disconnected ids: $disconnected_ids
