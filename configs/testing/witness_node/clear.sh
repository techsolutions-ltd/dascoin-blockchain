#!/bin/sh

DATA_FOLDER="witness_node_data_dir"
OBJECT_DATABASE="object_database"

echo "Clearing all working witness_node files and folders:";

if [ -d "$OBJECT_DATABASE" ]; then
  echo "Clearing folder $OBJECT_DATABASE";
  rm -r "$OBJECT_DATABASE";
else
  echo "WARNING: folder $OBJECT_DATABASE deoes not exist";
fi

cd "$DATA_FOLDER"
if [ $? -ne 0 ]; then
    echo "ERROR: data folder $DATA_FOLDER does not exist, stopping";
    return 1
fi

for i in "blockchain" "p2p" "logs"
do
    if [ -d $i ]; then
        echo "Removing $DATA_FOLDER/$i";
        rm -r "$i"
    else
        echo "WARNING: folder $DATA_FOLDER/$i does not exist"
    fi
done

for i in "db_version"
do
    if [ -f $i ]; then
        echo "Removing $DATA_FOLDER/$i";
        rm "$i";
    else
	echo "WARNING: $DATA_FOLDER/$i does not exist"
    fi
done

echo "Finished."
cd ..

