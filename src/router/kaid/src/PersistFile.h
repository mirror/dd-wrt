#ifndef PERSISTFILE_H
#define PERSISTFILE_H

#include <string>
#include <vector>
#include <map>

using namespace std;

typedef pair <string, string> PersistItemPair;

class CPersistFile {
	private:
		map <string, string> m_mPersistInfo;
		string 	m_sFilename;
		void		Clear();
		void		Read();
		void		Write();
	public:
		string	GetValue(string sUsername, string sKey);
		void		SetValue(string sUsername, string sKey, string sValue);
				CPersistFile(string sFilename);
				~CPersistFile();
};

#endif
