#include "zlib_inc.h"

bool gz_compress(FILE *in, gzFile out)
{
	char buf[BUFLEN];
	int len;

	for (;;) {
		len = (int)fread(buf, 1, sizeof(buf), in);
		if (ferror(in)) {
			return false;
		}
		if (len == 0) break;

		if (gzwrite(out, buf, (unsigned)len) != len) return false;
	}
	return true;
}


bool gz_uncompress(gzFile in, FILE *out)
{
	char buf[BUFLEN];
	int len;

	for (;;) {
		len = gzread(in, buf, sizeof(buf));
		if (len < 0) return false;
		if (len == 0) break;

		if ((int)fwrite(buf, 1, (unsigned)len, out) != len) {
			return false;
		}
	}
	return true;
}

bool zipAddFileToZip(zipFile zip, const char* newFilePath, FILE* fp)
{
	char buf[BUFLEN];
	int len;

	if (zipOpenNewFileInZip(zip, newFilePath, NULL, NULL, 0, 0, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION) != ZIP_OK) 
		return false;
	
	for (;;) {
		len = (int)fread(buf, 1, sizeof(buf), fp);
		if (ferror(fp)) {
			return false;
		}
		if (len == 0) break;

		if (zipWriteInFileInZip (zip,buf,len) < 0) return false;
	}

	if (zipCloseFileInZip(zip) != ZIP_OK) return false;
	return true;
}