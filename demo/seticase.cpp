#include <windows.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <ndk/ntndk.h>
#include <winutil.h>
#include <winutf8.h>
#include <string>
using std::string;

static int failcnt = 0;
static int verbose = 0;


bool set_case(const char * fn, bool cs)
{
	//printf("setting %s %d\n", fn, cs);
	FILE_CASE_SENSITIVE_INFORMATION ci;
	uint32_t err = 0;
	HANDLE hdir = CreateFileU(fn, FILE_READ_ACCESS|FILE_READ_ATTRIBUTES|STANDARD_RIGHTS_READ|
		FILE_READ_EA|FILE_WRITE_EA| FILE_WRITE_ATTRIBUTES,
		FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0);
	if (hdir && hdir != INVALID_HANDLE_VALUE)
	{
		IO_STATUS_BLOCK iob;
		NTSTATUS nt = NtQueryInformationFile(hdir, &iob, &ci, sizeof(ci), FileCaseSensitiveInformation);
		ci.Flags = cs ? FILE_CS_FLAG_CASE_SENSITIVE_DIR : 0;
		nt = NtSetInformationFile(hdir, &iob, &ci, sizeof(ci), FileCaseSensitiveInformation);
		CloseHandle(hdir);
		if (nt == STATUS_SUCCESS)
		{
			static_assert(sizeof("哈哈") == 7, "must be utf-8");
			if (verbose)
				fprintf(stderr, "set %s %s : OK\n", fn, cs ? "目录区分大小写" : "目录不区分大小写");
			return true;
		}
		err = RtlNtStatusToDosError(nt);
		char * es = win_strerror(err);
		fprintf(stderr, "set %s failed: %u,%s\n", fn, err, es);
		free(es);
		return false;
	}
	else
	{
		err = GetLastError();
		char * es = win_strerror(err);
		fprintf(stderr, "open %s failed: %u,%s\n", fn, err, es);
		free(es);
		return false;
	}
}

bool recurse_set(string& fn, bool cs)
{
	set_case(fn.c_str(), cs);
	DIR * d = opendir(fn.c_str());
	if (!d)
	{
		fprintf(stderr, "failed to opendir %s\n", fn.c_str());
		return false;
	}

	if (fn.c_str()[fn.length() - 1] != '\\')
		fn += '\\';

	for (;;)
	{
		struct dirent * dr = readdir(d);
		if (!dr) break;
		if (dr->d_type != DT_DIR) continue;
		if (dr->d_name[0] == '.' && (dr->d_name[1] == 0 || (
			dr->d_name[1] == '.' && dr->d_name[2] == 0)))
			continue;
		size_t len = fn.length();
		fn += dr->d_name;
		recurse_set(fn, cs);
		fn.resize(len);
	}
	closedir(d);
	return true;
}

int main(int argc, char ** argv, char ** envp)
{
	auto usage = [](){
		fprintf(stderr, "usage: seticase [-v] [-r] [-c] [-C] dir [dir...]\n");
	};
	char buf[900];
	DWORD xx = GetTempPathU(sizeof(buf), buf);

	int recurse = 0;
	int case_sensi = false;
	for (;;)
	{
		int op = getopt(argc, argv, "vrcC");
		if (op == -1) break;
		switch (op)
		{
		case 'v':
			verbose ++;
			break;
		case 'r':
			recurse = 1;
			break;
		case 'c':
			case_sensi = 0;
			break;
		case 'C':
			case_sensi = 1;
			break;
		default:
			return usage(), 1;
		}
	}
	argc -= optind;
	argv += optind;
	if (argc == 0)
		return usage(), 1;

	adjust_privilege(SE_BACKUP_NAME, 1);
	adjust_privilege(SE_RESTORE_NAME, 1);

	for (int i=0; i<argc; ++i)
	{
		//printf("%s\n", argv[i]);
		if (recurse)
		{
			string fn0(argv[i]);
			recurse_set(fn0, case_sensi);
		}
		else
			set_case(argv[i], case_sensi);
	}
	return 0;
}
