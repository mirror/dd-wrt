#include <iostream>
#include <fstream>
#include <socketcc.h>
#include "PersistFile.h"
#include "Kaid.h"
#include "StrUtils.h"

using namespace std;

CPersistFile::CPersistFile(string sFilename)
{
	m_sFilename=sFilename;
	Read();
}

CPersistFile::~CPersistFile()
{
}

void CPersistFile::Clear()
{
	m_mPersistInfo.clear();
}

string CPersistFile::GetValue(string sUsername, string sKey)
{
	map <string,string> :: iterator theItem;
	
	theItem=m_mPersistInfo.find(sUsername+(char)1+sKey);
	if(theItem==m_mPersistInfo.end())
		return "";
	else
		return theItem->second;
}

void CPersistFile::SetValue(string sUsername, string sKey, string sValue)
{
	map <string,string> :: iterator theItem;
	
	if(sValue.size()>0)
	{
		if(sValue[sValue.size()-1]!=';')
			sValue+=";";
	}
	theItem=m_mPersistInfo.find(sUsername+(char)1+sKey);
	if(theItem!=m_mPersistInfo.end())
	{
		if(sValue.size()>0)
			theItem->second=sValue;
		else
			m_mPersistInfo.erase(theItem);
	}
	else
	{
		if(sValue.size()>0)
			m_mPersistInfo.insert(PersistItemPair(sUsername+(char)1+sKey,sValue));
	}
	Write();
}

void	CPersistFile::Read()
{
	string sThisLine;
	string sUsername;
	string sKey;
	string sValue;
	ifstream fReader;
	
	fReader.open(m_sFilename.c_str());
	if(!fReader.is_open())
	{
		debuglog("WARNING","Unable to open engine persist data file (" + m_sFilename + ")");
		return;
	}
	int iFrom,iTo;
	
	getline(fReader,sThisLine);
	if(sThisLine.substr(0,18)=="KAI_ENGINE_PERSIST")
	{
		while(getline(fReader,sThisLine))
		{
			iTo=sThisLine.find(";");
			sUsername=sThisLine.substr(0,iTo);
			iFrom=iTo+1;
			iTo=sThisLine.find(";",iFrom);
			sKey=sThisLine.substr(iFrom,iTo-iFrom);
			iFrom=iTo+1;
			iTo=sThisLine.rfind(";");
			sValue=sThisLine.substr(iFrom,iTo-iFrom+1);
			
			SetValue(sUsername,sKey,sValue);
			
		}
	}	
	fReader.close();
}

void	CPersistFile::Write()
{
	map <string,string> :: iterator theItem;
	ofstream fWriter;

	fWriter.open(m_sFilename.c_str());
	if(!fWriter.is_open())
	{
		debuglog("WARNING","Unable to open engine persist data file for output - check permissions (" + m_sFilename + ")");
		return;
	}
	vector <string> vsTokens;
	fWriter << "KAI_ENGINE_PERSIST\n";
	for(theItem=m_mPersistInfo.begin(); theItem!=m_mPersistInfo.end(); theItem++)
	{
		Tokenize(theItem->first,vsTokens,(char)1);
		fWriter << (vsTokens[0] + ";" + vsTokens[1] + ";" + theItem->second + "\n").c_str();
	}
}
	
