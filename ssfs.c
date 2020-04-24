#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/stat.h>

static  const  char *dirpath = "/home/excel/Documents";
char encrypt[90] = "9(ku@AW1[Lmvgax6q`5Y2Ry?+sF!^HKQiBXCUSe&0M.b%rI'7d)o4~VfZ*{#:}ETt$3J-zpc]lnh8,GwP_ND|jO";


void encryption(char x[1000]) {
    if(!strcmp(x, ".") || !strcmp(x, "..")) return;
	int i, j, length, enc_length;
    length = strlen(x);
    enc_length = strlen(encrypt);
	for (i = 0; i < length; i++) {
		for (j = 0; j < enc_length; j++) {
			if(x[i] == encrypt[j]) {
				x[i] = encrypt[(j+10)%enc_length];
				break;
 			}
		}
	}
}

void decryption(char x[1000]) {
    if(!strcmp(x, ".") || !strcmp(x, ".."))return;
	int i, j, length, enc_length;
    length = strlen(x);
    enc_length = strlen(encrypt);
	for (i = 0; i < length; i++){
		for (j = 0; j < enc_length; j++){
		    if(x[i] == encrypt[j]){
                x[i] = encrypt[(j-10) % enc_length];
                break;
 			}
		}
	}
}

static  int  xmp_getattr(const char *path, struct stat *stbuf)
{
    int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);
	res = lstat(fpath, stbuf);
	if (res == -1)
		return -errno;
	return 0;

}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    char fpath[1000];
    if(strcmp(fpath,"/") == 0){
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else{
        sprintf(fpath, "%s/%s", dirpath, path);
    }

	DIR *dp;
	struct dirent *de;
	(void) offset;
	(void) fi;
	
	char xpath[1000];
    int enc_type=0;
	sprintf(xpath, "%s", path);

	if(strstr(xpath, "encv1_")){
		printf("First Encryption Mode\n");
		enc_type = 1;
	}
	else if (strstr(xpath, "encv2_")){
		enc_type = 2;
    }
	printf("readdir [%s]\n", fpath);

	dp = opendir(fpath);
	if (dp == NULL)
		return -errno;

    char enc[1000];
	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		sprintf(enc, "%s", de->d_name);
		if(enc_type == 1){
			encryption(enc);
		}
        /*else if(enc_type == 2){
            encryption2(enc);
        }
        */
		printf("readdir %s | %s\n", xpath, enc);
		if (filler(buf, enc, &st, 0))
			break;
	}
    closedir(dp);
    return 0;
}

static int xmp_mkdir(const char *path, mode_t mode) {
	int res;
    char fpath[1000];
    sprintf(fpath,"%s%s", dirpath, path);
    printf("fpath mkdir %s\n", fpath);
	res = mkdir(fpath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	if (res == -1)
		return -errno;
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    char fpath[1000];
    if(strcmp(path,"/") == 0) {
        path=dirpath;
        sprintf(fpath, "%s", path);
    }
    else sprintf(fpath, "%s%s", dirpath, path);

    int fd;
    int res;
    (void) fi;

    fd = open(fpath, O_RDONLY);
    if (fd == -1) 
        return -errno;

    res = pread(fd, buf, size, offset);
    if (res == -1) 
        res = -errno;

    close(fd);
    return res;
}

static int xmp_utimens(const char *path, const struct timespec ts[2]) {
	int res;
	struct timeval tv[2];

    char fpath[1000];
    sprintf(fpath,"%s%s", dirpath, path);
    printf("fpath create %s\n", fpath);

	tv[0].tv_sec = ts[0].tv_sec;
	tv[0].tv_usec = ts[0].tv_nsec / 1000;
	tv[1].tv_sec = ts[1].tv_sec;
	tv[1].tv_usec = ts[1].tv_nsec / 1000;

	res = utimes(fpath, tv);
	if (res == -1) 
        return -errno;

	return 0;
}

static int xmp_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {

	int res;
    int fd;
    char fpath[1000];

    if(strcmp(path,"/") == 0) {
        path=dirpath;
        sprintf(fpath, "%s", path);
    }
    else sprintf(fpath, "%s%s", dirpath, path);

	(void) fi;
	fd = open(fpath, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1) 
        res = -errno;

	close(fd);
	return res;
}

static int xmp_rename(const char *from, const char *to) {
    int res;

	res = rename(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_unlink(const char *path)
{
	printf("unlink\n");
	int res;

	res = unlink(path);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rmdir(const char *path)
{
	int res;

	res = rmdir(path);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
	int res;
	res = open(path, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}

static int xmp_access(const char *path, int mask)
{
	int res;

	res = access(path, mask);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readlink(const char *path, char *buf, size_t size)
{
	int res;

	res = readlink(path, buf, size - 1);
	if (res == -1)
		return -errno;

	buf[res] = '\0';
	return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res;

	/* On Linux this could just be 'mknod(path, mode, rdev)' but this
	   is more portable */
	if (S_ISREG(mode)) {
		res = open(path, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	} else if (S_ISFIFO(mode))
		res = mkfifo(path, mode);
	else
		res = mknod(path, mode, rdev);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_symlink(const char *from, const char *to)
{
	printf("symlink\n");
	int res;

	res = symlink(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_link(const char *from, const char *to)
{
	printf("link\n");
	int res;

	res = link(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chmod(const char *path, mode_t mode)
{
	int res;

	res = chmod(path, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chown(const char *path, uid_t uid, gid_t gid)
{
	int res;

	res = lchown(path, uid, gid);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_truncate(const char *path, off_t size)
{
	int res;

	res = truncate(path, size);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_statfs(const char *path, struct statvfs *stbuf)
{
	int res;

	res = statvfs(path, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_create(const char* path, mode_t mode, struct fuse_file_info* fi) {

    (void) fi;

    int res;
    res = creat(path, mode);
    if(res == -1)
	return -errno;

    close(res);

    return 0;
}

static int xmp_release(const char *path, struct fuse_file_info *fi)
{
	(void) path;
	(void) fi;
	return 0;
}

static int xmp_fsync(const char *path, int isdatasync,
		     struct fuse_file_info *fi)
{
	(void) path;
	(void) isdatasync;
	(void) fi;
	return 0;
}


static struct fuse_operations xmp_oper = {

    .getattr	= xmp_getattr,
	.access		= xmp_access,
	.readlink	= xmp_readlink,
	.readdir	= xmp_readdir,
	.mknod		= xmp_mknod,
	.mkdir		= xmp_mkdir,
	.symlink	= xmp_symlink,
	.unlink		= xmp_unlink,
	.rmdir		= xmp_rmdir,
	.rename		= xmp_rename,
	.link		= xmp_link,
	.chmod		= xmp_chmod,
	.chown		= xmp_chown,
	.truncate	= xmp_truncate,
	.utimens	= xmp_utimens,
	.open		= xmp_open,
	.read		= xmp_read,
	.write		= xmp_write,
	.statfs		= xmp_statfs,
	.create     = xmp_create,
	.release	= xmp_release,
	.fsync		= xmp_fsync,
}; 

int main(int  argc, char *argv[]) {
    umask(0);
    return fuse_main(argc, argv, &xmp_oper, NULL);

}
