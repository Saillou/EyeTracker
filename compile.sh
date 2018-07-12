#!/bin/bash

# Functions
create() {
	if [ ! -d $1 ]
	then
		mkdir $1
	fi	
}
cleanup() {
	if [ -f $1 ]
	then
		rm $1
	fi	
}
makePathSources() {
	# Sources
	for source in "${Sources[@]}"
	do
		if [ -f "../../Sources/$source.cpp" ]
		then
			PathSources="$PathSources ../../Sources/$source.cpp"
		else
			echo "$source.cpp is missing"
			return 0
		fi
	done
	
	# Continue ? 	
	if [ $compileDk == 0 ]
		then return 1
	fi
	
	# Sources of Lib Dk
	for source in "${Dk[@]}"
	do
		if [ -f "../../Sources/Dk/$source.cpp" ]
		then
			PathSources="$PathSources ../../Sources/Dk/$source.cpp"
		else
			echo "Dk/$source.cpp is missing"
			return 0
		fi
	done 
	
	return 1
}
makePathObjects() {
	# Objects from source
	for source in "${Sources[@]}"
	do
		if [ -f "../Objects/$source.o" ]
		then
			PathObjects="$PathObjects ../Objects/$source.o"
		else
			echo "$source.o is missing"
			return 0
		fi
	done 
	
	# Objects from Dk
	for source in "${Dk[@]}"
	do
		if [ -f "../Objects/$source.o" ]
		then
			PathObjects="$PathObjects ../Objects/$source.o"
		else
			echo "$source.o is missing"
			return 0
		fi
	done 
	
	return 1	
}


# Define environnement
create "Ubuntu"

cd "Ubuntu"
create "Objects"
create "Release"

OutputName="EyeTracker"
declare -a Sources=(
	"main_ubuntu" 
)
declare -a Dk=(
	"ManagerConnection" 
	"Protocole" 
	"Server" 
	"Socket"
)
declare compileDk=1
declare fetchGithub=0

PathSources=""
PathObjects=""

# Get update from github
if [ fetchGithub == 1 ]
	then git pull EyeTracker master
fi


# Compile source
cd "Objects"
cleanup *.o
makePathSources
if [ $? == 0 ]
	then exit -1
fi

echo "Compile sources"
g++ -c -std=gnu++11 -O2 \
	-Wall \
	$PathSources \
	-I/usr/local/include/  \
	-I/usr/local/lib/libjpeg/
cd ../


# Link objects
cd "Release"
cleanup $OutputName 
makePathObjects
if [ $? == 0 ]
	then exit -1
fi

echo "Link objects"
g++ -o $OutputName \
	$PathObjects \
	-L/usr/local/lib/ \
	-L/usr/local/lib/libjpeg/ \
	-pthread \
	-lopencv_core \
	-lturbojpeg \
	-lopencv_highgui \
	-lopencv_imgproc \
	-lopencv_imgcodecs \
	-lopencv_videoio \


# Launch
echo "-----------"
./$OutputName

exit 0
