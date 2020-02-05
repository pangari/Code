#include "glstring.H"

#include <cstdio>
#include <cctype>
#include <cstdarg>
#include <functional>
#include <algorithm>
#include <iostream>

#ifdef WIN32
#define vsnprintf _vsnprintf
#endif

using namespace std;

bool GLString::ExtendCompare(const string &str1,const string &str2 )
{
	const char * _str1 = str1.c_str();
	const char * _str2 = str2.c_str();
	for(;*_str1!='\0' && *_str2!='\0';_str1++,_str2++) {
		if(*_str1=='*' || *_str2=='*') return 1;
		if(*_str1!='?' && *_str2!='?' && *_str1!=*_str2)
			return 0;
	}
	return (*_str1==*_str2);
 }


string GLString::ToUpper(const string &src)
{
	string result;
	result.reserve(src.size());
	transform(src.begin(),src.end(),inserter(result,result.begin()),ptr_fun(::toupper));
	return result;
}

string GLString::ToLower(const string &src)
{
	string result;
	result.reserve(src.size());
	transform(src.begin(),src.end(),inserter(result,result.begin()),ptr_fun(::tolower));
	return result;
}


class IsLowerEqualTo:public binary_function<char,char,bool> {
public:
	bool operator()(char c1,char c2) const {
		return ::tolower(c1)==::tolower(c2);
	}
};

string::size_type GLString::Index(const string &src,const string &substr, bool sensitive)
{
	string::const_iterator found;
	if (sensitive)
		found=search(src.begin(),src.end(),substr.begin(),substr.end());
	else
		found=search(src.begin(),src.end(),substr.begin(),substr.end(),IsLowerEqualTo());
	if (found==src.end())
		return string::npos;
	return found-src.begin();
}

string::size_type  GLString::IndexFromEnd(const string &src,const string &substr, bool sensitive)
{
	string::const_reverse_iterator found;
	if (sensitive)
		found=search(src.rbegin(),src.rend(),substr.rbegin(),substr.rend());
	else
		found=search(src.rbegin(),src.rend(),substr.rbegin(),substr.rend(),IsLowerEqualTo());
	if (found==src.rend())
		return string::npos;
	string::size_type delta=found-src.rbegin();
	return src.size()-substr.size()-delta;
}


string GLString::Strip(const std::string &src, const char stripchar, char side )
{
	string::const_iterator begin=
		(side==' ' || side=='L')?
		find_if(src.begin(),src.end(),bind2nd(not_equal_to<char>(),stripchar)):
		src.begin();
	string::const_reverse_iterator end=
		(side==' ' || side=='T')?
		find_if(src.rbegin(),src.rend(),bind2nd(not_equal_to<char>(),stripchar)):
		src.rbegin();
	string::size_type posBegin=begin-src.begin();
	string::size_type posEnd=src.size()-(end-src.rbegin());
	string result;
	if (posEnd-posBegin>0){
		result = string(src,posBegin,posEnd-posBegin);
	}
	return result;
}


string GLString::Parse(string &right,const string &separator, bool sensitive, bool stripwsp )
{
    // if this is NULL, returns a NULL string
    if ( right.empty() || separator.empty() )
        return string();

    // if the separator is in the calling string
    string::size_type i= Index(right, separator, sensitive );
	std::string left(right, 0, i );
	if ( stripwsp ) left=Strip(left);
    if ( i  != string::npos ) {
        right = string(right, i+separator.size() );
        if ( stripwsp ) right=Strip(right);
    } else {
        // else, returns the remaining string and empty this
        right.clear();
    }
	return left;
}

string GLString::ParseFromEnd( string &right, const string &separator, bool sensitive, bool stripwsp )
{
    // if this is NULL, returns a NULL string
    if ( right.empty() || separator.empty() )
        return string();

    // if the separator is in the calling string
    string::size_type i= IndexFromEnd(right, separator, sensitive );
	std::string left(right, 0, i );
	if ( stripwsp ) left=Strip(left);
    if ( i  != string::npos ) {
        right.erase(0, i+separator.size() );
        if ( stripwsp ) right=Strip(right);
    } else {
        // else, returns the remaining string and empty this
        right.clear();
    }
	return left;
}

int GLString::Replace( string &src, const string & oldStr, const string & newStr, bool sensitive )
{
	int count = 0;
	string::iterator found(src.begin());
	for (;;) 
	{
		if (sensitive)
			found=search(found,src.end(),oldStr.begin(),oldStr.end());
		else
			found=search(found,src.end(),oldStr.begin(),oldStr.end(),IsLowerEqualTo());
		if (found==src.end())
			break;
		string::size_type start=found-src.begin();
		src.replace(start,oldStr.size(),newStr);
		found=src.begin()+start+newStr.size();
		count++;
	}
	return count;
}


string GLString::Overlay(const string &src, const string &subStr, string::size_type position, string::size_type len )
{
	string result(src);
	string::size_type srcLen=(len==string::npos?subStr.size():len);
	result.replace(position,srcLen,subStr,0,len);
	return result;
}

class NotInString
{
	const string &m_str; // warning store a ref (use only in this module)
public:
	NotInString(const string &str):m_str(str) {}
	bool operator()(char c) {
		return find(m_str.begin(),m_str.end(),c)==m_str.end();
	}
};

string::size_type GLString::WordIndex(const string &src, string::size_type n,const string & separator )
{
	string::const_iterator i=find_if(src.begin(),src.end(),
									 NotInString(separator));
	while (i!=src.end() && n>0) {
		i=find_first_of(i,src.end(),separator.begin(),separator.end());
		i=find_if(i,src.end(),NotInString(separator));
		n--;
	}
	if (i==src.end()) return string::npos;
	return i-src.begin();
}


string GLString::Word(const string &src, string::size_type n, const string &separator )
{
	string::size_type i=WordIndex(src,n,separator);
	if (i==string::npos) return string();
	string::const_iterator begin=src.begin()+i;
	string::const_iterator end=find_first_of(begin,src.end(),
											 separator.begin(),separator.end());
	return string(src,i,end-begin);
}



string GLString::Extract(const string &src, string::size_type n, const string &separator )
{
	if (n==0) return string();
	string::const_iterator i=src.begin();
	n--;
	while (i!=src.end() && n>0) {
		i=find_first_of(i,src.end(),separator.begin(),separator.end());
        i++;
		n--;
	}
	return string(i,find_first_of(i,src.end(),separator.begin(),separator.end()));
}

string Format( const char *fmt, ... )
{
    char buf[ 16384 ];
	int len;
    va_list marker;
    va_start( marker, fmt );
    len = vsnprintf(buf,16384, fmt, marker );
	va_end( marker);
    return string(buf,0,len);
}



GLStringList *GLString::Split(const char * _c, bool _sensitive , bool _stripwsp)
{
    GLStringList *_res = new GLStringList;

    GLString v_tmp(*this);
    GLString v_tmp1;

    while(v_tmp.Length()) /* keep Length() => if r == 0 ) */
    {
        v_tmp1 = v_tmp.Parse(_c, _sensitive, _stripwsp);
        _res->push_back(v_tmp1);
    }
    return _res;

}

GLStringList *GLString::Split(const GLString& _c, bool _sensitive, bool _stripwsp)
{
    GLStringList *_res = new GLStringList;

    GLString v_tmp(*this);
    GLString v_tmp1;

    while(v_tmp.Length()) /* keep Length() => if r == 0 ) */
    {
        v_tmp1 = v_tmp.Parse(_c, _sensitive, _stripwsp);
        _res->push_back(v_tmp1);
    }
    return _res;

}



GLStringList *GLString::Split(const char _c, bool _sensitive, bool _stripwsp)
{
    GLStringList *_res = new GLStringList;

    GLString v_tmp(*this);
    GLString v_tmp1;


    while(v_tmp.Length()) /* keep Length() => if r == 0 ) */
    {
        v_tmp1 = v_tmp.Parse(_c, _sensitive, _stripwsp);
        if (v_tmp1.Length() == 0)  /* keep Length() => if r == 0 ) */
        {
            v_tmp1 = "";
        }
        _res->push_back(v_tmp1);
    }

    return _res;
}

void GLString::_TestError(const char *msg)
{
	cerr<<"GLString Test Error: "<<msg<<endl;
}

bool GLString::Test()
{
	GLString str1("Hello World");
	GLString str2("Hello Everybody");
	if (str1.Compare(str1)!=0) _TestError("Compare =");
	if (str1.Compare(str2)!=1) _TestError("Compare >");
	if (str2.Compare(str1)!=-1) _TestError("Compare <");
	if (str1.Length()!=strlen(str1.c_str())) _TestError("Length");
	str1="ABCDEF";
	GLString leftResult("ABCD");
	GLString rightResult("CDEF");
	GLString subResult("CD");
	if (str1.Left(4)!=leftResult) _TestError("Left");
	if (str1.Right(4)!=rightResult) _TestError("Right");
	if (str1.SubString(2,2)!=subResult) _TestError("CD");
	GLString errStr;
	errStr="ExtendCompare";
	if (GLString("ABC*").ExtendCompare("ABCDEF")==0) _TestError(errStr.c_str());
	if (GLString("AB?D").ExtendCompare("ABCD")==0) _TestError(errStr.c_str());
	if (GLString("A*xxx").ExtendCompare("AB*yyyy")==0) _TestError(errStr.c_str());
	if (GLString("ABCD?FG").ExtendCompare("A?CDEFx")==1) _TestError(errStr.c_str());
	if (GLString("WoRlD").ToUpper()!=GLString("WORLD")) _TestError("ToUpper");
	if (GLString("WoRlD").ToLower()!=GLString("world")) _TestError("ToLower");
	errStr="Index";
	if (GLString("WorldWorld").Index("rl")!=2) _TestError(errStr.c_str());
	if (GLString("WorldWorld").Index("rd")!=(unsigned int)(-1)) _TestError(errStr.c_str());
	if (GLString("WoRldWoRld").Index("rl")!=(unsigned int)(-1)) _TestError(errStr.c_str());
	if (GLString("WoRldWoRld").Index("rl",false)!=2) _TestError(errStr.c_str());
	errStr="IndexFromEnd";
	if (GLString("WorldWorld").IndexFromEnd("rl")!=7) _TestError(errStr.c_str());
	if (GLString("WorldWorld").IndexFromEnd("rd")!=(unsigned int)(-1)) _TestError(errStr.c_str());
	if (GLString("WoRldWoRld").IndexFromEnd("rl")!=(unsigned int)(-1)) _TestError(errStr.c_str());
	if (GLString("WoRldWoRld").IndexFromEnd("rl",false)!=7) _TestError(errStr.c_str());
	str1=" ABCD = 1234 ";
	str2=str1.Parse("=");
	if (str2!="ABCD" || str1!="1234") _TestError("Parse");
	str1="  Hello  ";
	str1.Strip();
	if (str1!="Hello") _TestError("Strip");
	str1="AB CD EF AB";
	str1.Replace( "AB", "XX" );
	if (str1!="XX CD EF XX") _TestError("Replace");
	str1="ABCDEF";
	str1.Overlay("XX",2);
	if (str1!="ABXXEF") _TestError("Overlay");
	str1="AB\t\tCD\tEF";
	if (str1.Extract(2) != "") _TestError("Extract");
	if (str1.Extract(3) != "CD") _TestError("Extract");
	str1="AB  CD  EF";
	if (str1.WordIndex(1) !=4) _TestError("WordIndex");
	str1="AB XX CD XX EF";
	if (str1.WordIndex( 1, " X" )!=6) _TestError("WordIndex");
	str1="lookup(1001)";
	if (str1.Word(0," ,()")!="lookup") _TestError("Word");
	str1="AB  CD  EF";
	if (str1.Word(1)!="CD") _TestError("Word");
	str1="AB XX CD XX EF";
	if (str1.Word(1," X")!="CD") _TestError("Word");
	str1="   AB   ";
	if (str1.Strip()!="AB") _TestError("Strip");
	str1="this.is.a.string.to.split";
	GLStringList *l=str1.Split(".");
	if (l->size()!=6) _TestError("Split");
	delete l;
	if (GLString("14").AsInt()!=14) _TestError("AsInt");
	if (GLString("-14").AsLong()!=-14L) _TestError("AsLong");
	if (GLString("-14.3").AsDouble()!=-14.3) _TestError("AsDouble");
	if (Format("%d %s",10,"-").c_str()!=GLString("10 -")) _TestError("Format");
	return true;
}

