/**
 * INI��ȡ����
 * Xu Lubing
 * Jan. 22, 2007
 *
 * INI�ļ���ʽ:
 * [section]
 * # ע����
 * key1 = value1 # ��������βע�͡�����ͨ��get_string/get_int�Ⱥ�����ȡkey��ֵ
 * key2 = value2 # ����key
 * ...           # ����section�ķ�ע���п���ͨ��get_section���
 *
 * [����section]
 * ...
 */
#ifndef _INI_PARSER_H_
#define _INI_PARSER_H_

#include <string>
#include <map>
#include <set>
namespace utility
{

typedef std::map<std::string, std::string> stringMap; // ����key => value
typedef std::map<std::string, stringMap> sectionMap;  // ����section => key/value
typedef std::set<std::string> sectionSet;
class INIParser
{
public:
	/**
	 * ���캯��
	 * @param profile  INI�ļ���
	 */
	INIParser();
	INIParser(const char *profile);
	INIParser(const std::string &profile);

	virtual ~INIParser();

	/**
	 * ���������ļ�(ͬʱ���ԭ�е�������Ϣ)
	 * @param profile  INI�ļ���
	 */
	void load(const char *profile);
	void load(const std::string &profile);

	/**
	 * ���ַ�����Ϊ�������ݣ���������������ͬʱ����ԭ�е�������Ϣ
	 * @param content  ��������
	 */
	void parse(const char *content);
	void parse(const std::string &content);

	/**
	 * ��ȡһ���ַ���
	 * @param section  section��
	 * @param key      ������
	 * @param def      ������������ʱ��ȱʡֵ
	 * @return ָ��������ֵ
	 */
	const std::string &get_string(const std::string &section, const std::string &key, const std::string &def)const;
	const char *get_string(const char *section, const char *key, const char *def = NULL)const;

	/**
	 * ��ȡһ������ֵ
	 * @param section  section��
	 * @param key      ������
	 * @param def      ������������ʱ��ȱʡֵ
	 * @return ָ��������ֵ
	 */
	int get_int(const std::string &section, const std::string &key, int def = 0)const;
	int get_int(const char *section, const char *key, int def = 0)const;

	/**
	 * ��ȡһ���Ǹ�����ֵ
	 * @param section  section��
	 * @param key      ������
	 * @param def      ������������ʱ��ȱʡֵ
	 * @return ָ��������ֵ
	 */
	unsigned get_unsigned(const std::string &section, const std::string &key, unsigned def = 0)const;
	unsigned get_unsigned(const char *section, const char *key, unsigned def = 0)const;

	/**
	 * ��ȡһ������ֵ������ֵ���ַ���Ϊ"Y"/"YES"/"T"/"True"��Ϊ�棬����Ϊ��
	 * @param section  section��
	 * @param key      ������
	 * @param def      ������������ʱ��ȱʡֵ
	 * @return ָ��������ֵ
	 */
	bool get_bool(const std::string &section, const std::string &key, bool def = false)const;
	bool get_bool(const char *section, const char *key, bool def = false)const;

	/**
	 * �޸�һ�����ֵ������������޸ģ������ڷ��ط�0
	 * @param section section��
	 * @param key     ������
	 * @param val     �޸ĺ�ı���ֵ
	 * @return        �޸ĳɹ�0��ʧ�ܷ�0
	 */
	int change(const std::string &section, const std::string &key, const std::string &val);
	int change(const char *section, const char *key, const char *val);

	/**
	 * ��ȡĳһ��section����
	 * @param section   ָ����section��
	 * @return ��Ӧ������
	 */
	std::string &get_section(const std::string &section);
	const char *get_section(const char *section);

	void get_sections(sectionSet &sets);
	const stringMap *get_section_map(const std::string &section);
	const stringMap *get_section_map(const char *section);

	/**
	 * ����ini�����ݣ�������
	 */
	void dump()const;
private:
	sectionMap m_ini;
	stringMap m_sections;
	std::string m_none;
	std::string NOTHING;

	bool find_section(const char *p, std::string &section, std::string &sq_section, std::string &sec_body);
	const char *find_pair(const char *p, std::string &section);
	void append_sq_section(std::string &sq_section, std::string &sec_body, const char *p, int len=-1);
	void clear();
	void load(std::istream &f);
};

}

#endif

