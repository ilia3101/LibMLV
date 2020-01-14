get_libraw()
{
	git clone https://github.com/LibRaw/LibRaw.git -b 0.19-stable;
	cd LibRaw;
	make -f makefile.dist;
	mv lib/libraw_r.a ../libraw_r.a;
	cd ../;
}

# Git clone libraw so we have libraw headers
if [ ! -d "LibRaw" ]; then
	echo "Libraw is not present."
	read -p "Clone and compile libraw source code from Github? [y/n] " yn
	case $yn in
		[Yy]* )
			get_libraw;;
		[Nn]* ) echo "Ok bye you can't compile raw2mlv then."; exit;;
		* ) exit;;
	esac
fi

if [ ! -e libraw_r.a ]; then
	echo "Libraw is not present."
	read -p "Clone and compile libraw source code from Github? [y/n] " yn
	case $yn in
		[Yy]* )
			get_libraw;;
		[Nn]* ) echo "Ok bye you can't compile raw2mlv then."; exit;;
		* ) exit;;
	esac
else
	echo "libraw_r.a exists"
fi
