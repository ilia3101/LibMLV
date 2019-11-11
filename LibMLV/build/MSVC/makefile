CC=CL
FLAGS=/c /Ox

main: MLVWriter MLVReader MLVFrameUtils
	LIB *.obj /OUT:LibMLV.lib
	LINK /DLL *.obj /OUT:LibMLV.dll

MLVWriter:
	$(CC) ../../MLVWriter.c $(FLAGS)

MLVReader:
	$(CC) ../../MLVReader.c $(FLAGS)

MLVFrameUtils:
	$(CC) ../../MLVFrameUtils.c $(FLAGS)