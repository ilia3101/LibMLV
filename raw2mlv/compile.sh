# Git clone libraw so we have libraw headers
if [ ! -d "LibRaw" ]; then
	git clone https://github.com/LibRaw/LibRaw.git -b 0.19-stable
fi

rm *.o

# compile raw2mlv
gcc -c -O3 raw2mlv.c

#compile libMLV
cd ../LibMLV/build/unix
make -j4
cd -
cp ../LibMLV/build/unix/libMLV.a libMLV.a

#get libraw
if [ ! -e libraw_r.a ]; then
	if [[ "$OSTYPE" == "linux-gnu" ]]; then
		echo "libraw_r.a is not present, add libraw_r.a to this folder (version 0.19.* please)"
		echo "Compile instructions: https://www.libraw.org/docs/Install-LibRaw.html"
		exit
	elif [[ "$OSTYPE" == "darwin"* ]]; then
		echo "libraw_r.a is not present"
		read -p "Download libraw_r.a? [y/n] " yn
		case $yn in
			[Yy]* )
				echo "Downloading libraw_r.a ..."
				wget -O ./libraw.zip https://www.libraw.org/data/LibRaw-0.19.4-MacOSX.zip &> /dev/null
				unzip libraw.zip &> /dev/null
				librawfolder=LibRaw-0.19.4
				cp ./$librawfolder/lib/libraw_r.a ./
				rm libraw.zip &> /dev/null
				rm -rf $librawfolder &> /dev/null
				echo "Downloaded"
				break;;
			[Nn]* ) echo "Put libraw_r.a in this folder thanks bye."; exit;;
			* ) exit;;
		esac
	else
		death "Unknown OS"
		exit;
	fi
else
	echo "libraw_r.a exists"
fi

#link
if [[ "$OSTYPE" == "darwin"* ]]; then # (must add -lstdc++ only because libraw)
	gcc libraw_r.a libMLV.a *.o -o raw2mlv -lm -lstdc++ #macos version
else
	gcc *.o libraw_r.a libMLV.a -o raw2mlv -lm -lgomp -lstdc++ #linux version
fi
rm *.o
