#include <windows.h>
#include <winutf8.h>
#include <unistd.h>
#include <stdio.h>
#include <glob.h>
#include <sys/stat.h>

#include <vector>
#include <string>
using std::vector;
using std::string;

bool s_end_with(const string& s, const char * xx)
{
	size_t xlen = strlen(xx);
	if (s.length() < xlen) return false;
	return strcmp(s.c_str() + s.length() - xlen, xx) == 0;
}

bool is_dir(const string & s)
{
	DWORD xx = GetFileAttributesU(s.c_str());
	if (xx == INVALID_FILE_ATTRIBUTES) return false;
	return xx & FILE_ATTRIBUTE_DIRECTORY;
}

bool find_vs_dir(string &vsd, string& sdkd)
{
	char * prog = getenv("ProgramFiles(x86)");
	if (!prog || !*prog) prog = getenv("ProgramFiles");
	if (!prog || !*prog) return false;
	char * d0 = 0;
	glob_t gl = {0};
	asprintf(&d0, "%s/Microsoft Visual Studio/20*/Professional/VC/Tools/MSVC/*/bin/Hostx64/x64/cl.exe", prog);
	glob(d0, 0, 0, &gl);
	for (size_t i=0; i<gl.gl_pathc; ++i)
	{
		const char * fd = strstr(gl.gl_pathv[i], "/bin");
		vsd.assign(gl.gl_pathv[i], fd-gl.gl_pathv[i]);
	}
	free(d0);
	globfree(&gl);

	asprintf(&d0, "%s/Windows Kits/10/Include/*", prog);
	glob(d0, 0, 0, &gl);
	for (size_t i=0; i<gl.gl_pathc; ++i)
	{
		string dir = gl.gl_pathv[i];
		if (is_dir(dir + "/ucrt") && is_dir(dir + "/shared") && is_dir(dir + "/um"))
		{
			sdkd = dir;
		}
	}
	free(d0);
	globfree(&gl);
	return !vsd.empty() && !sdkd.empty();
}

string merge_env(const char * name, vector<string>& xx)
{
	string path1;
	for (int i=0; i<(int)xx.size(); ++i)
	{
		path1 += xx[i];
		path1 += ';';
		//printf("%s\n", xx[i].c_str());
	}
	const char * en = getenv(name);
	if (en && *en)
		path1 += en;
	if (path1.back() == ';')
		path1.resize(path1.size()-1);
	return path1;
}

int main(int argc, char ** argv)
{
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
	string vsd, sdkd;
	if (!  find_vs_dir(vsd, sdkd))
	{
		fprintf(stderr, "cant find vs dir!\n");
		return 3;
	}

	vector<string> vpath;
	vpath.push_back(vsd + "\\bin\\Hostx64\\x64");
	string path = merge_env("path", vpath);
	SetEnvironmentVariableA("path", path.c_str());

	vector<string> vlibs;
	vlibs.push_back(vsd+"\\ATLMFC\\lib\\x64");
	vlibs.push_back(vsd+"\\lib\\x64");
	{
		string q = sdkd;
		size_t np = q.find("Include");
		q.resize(np);
		q+="lib";
		q += sdkd.c_str()+np+7;
		vlibs.push_back(q + "\\ucrt\\x64");
		vlibs.push_back(q + "\\um\\x64");
	}
	string lib = merge_env("lib", vlibs);
	SetEnvironmentVariableA("lib", lib.c_str());

	vector<string> vincs;
	vincs.push_back(vsd+"\\ATLMFC\\include");
	vincs.push_back(vsd+"\\include");
	vincs.push_back(sdkd+"\\ucrt");
	vincs.push_back(sdkd+"\\shared");
	vincs.push_back(sdkd+"\\um");
	vincs.push_back(sdkd+"\\winrt");
	vincs.push_back(sdkd+"\\cppwinrt");
	string inc = merge_env("include", vincs);
	SetEnvironmentVariableA("include", inc.c_str());

	string cmdline;
	string prog = "link.exe";
	bool haslogo = true;
	for (int i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "-nologo") == 0 ||
			strcmp(argv[i], "/nologo") == 0 ||
			0)
		{
			haslogo = false;
			break;
		}
	}
	for (int i=1; i<argc; ++i)
	{
		if (strcmp(argv[i], "-c")==0 ||
			strcmp(argv[i], "/c") == 0 ||
			strcmp(argv[i], "/E") == 0 ||
			strcmp(argv[i], "-E") == 0 ||
			s_end_with(argv[i], ".cpp") ||
			s_end_with(argv[i], ".c") ||
			0)
		{
			prog = "cl.exe";
			break;
		}
	}

	for (int i=1; i<argc; ++i)
	{
		prog += " ";
		if (strchr(argv[i], ' '))
		{
			prog += '"';
			prog += argv[i];
			prog += '"';
		}
		else
			prog += argv[i];
		prog += ' ';
		if (i == 1 && haslogo)
			prog += "/nologo ";
	}
	fflush(stdout);
	fflush(stderr);
	STARTUPINFOA sinfo = {sizeof(sinfo)};
	PROCESS_INFORMATION pinfo = {0};
	if ( CreateProcessA(NULL, &prog[0], 0,0,0,0,0,0,&sinfo, &pinfo) )
	{
		WaitForSingleObject(pinfo.hProcess, INFINITE);
		DWORD dwc = -1;
		GetExitCodeProcess(pinfo.hProcess, &dwc);
		return dwc;
	}
	return 1;
}
