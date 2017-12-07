
#include "INIParser.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

using namespace std;

namespace utility
{
bool INIParser::find_section(const char *p, string &section, string &sq_section, string &sec_body)
{
    if (*p != '[') return false;

    // possible section begins
    const char *sec_head = p;
    p++;
    while (*p && *p != '#' && *p != ']') p++;
    if (*p == ']') {
        // possible section found
        const char *sec_end = p;
        p++;

        while (*p && isspace(*p)) p++;
        if (*p == '\0' || *p == '#') {
            // a real section found
            if (!sq_section.empty()) {
                // save the old section body
                m_sections[sq_section] = sec_body;
                sec_body.clear();
            }
            sq_section.assign(sec_head, sec_end-sec_head+1);
            sec_head++;
            section.assign(sec_head, sec_end-sec_head);
            return true;
        }
    }

    return false;
}

void INIParser::append_sq_section(string &sq_section, string &sec_body, const char *p, int len)
{
    if (len < 0) {
        char *end = strchr((char*)p, '#');
        if (end != NULL)
            len = end - p;
        else
            len = strlen(p);
    }
    // trim tailing blanks
    for (--len; len>=0 && isspace(p[len]); len--);
    len++;

    if (len <= 0) return;
    sec_body.append(p, len);
    sec_body.append("\n");
}

const char *INIParser::find_pair(const char *p, string &section)
{
    if (*p == '=') {
        // no name given, goto the end of line
        while (*p && *p != '#') p++;
        return p;
    }

    const char *name_head = p;
    p++;
    while (*p && *p != '#' && *p != '=') p++;
    if (*p == '#' || *p == '\0')
        return p;

    if (*p == '=') {
        // name found
        const char *name_end = p - 1;
        p++;

        while (name_end > name_head && isspace(*name_end)) name_end--;
        name_end++;

        if (name_end == name_head) {
            // goto the end of line
            while (*p && *p != '#') p++;
            return p;
        }

        // save the name
        string name(name_head, name_end-name_head);

        // begin to find value
        string value;

        while (*p && isspace(*p)) p++;
        if (*p) {
            const char *value_head = p;
            p++;
            while (*p && *p != '#') p++;
            for (p--; p>value_head && isspace(*p); p--);
            p++;

            if (p > value_head)
                value.assign(value_head, p-value_head);
        }

        m_ini[section][name] = value;
        return p;
    }

    // find the end of line
    while (*p && *p != '#') p++;
    return p;
}

INIParser::INIParser():m_none("{[NONE]}"),NOTHING("")
{
}

void INIParser::clear()
{
    m_ini.clear();
    m_sections.clear();
}

void INIParser::load(std::istream &f)
{
    string section;
    string sq_section;
    string sec_body;

    string line;
    while (f.good() && getline(f, line)) {
        if (line.empty()) continue;

        const char* p = line.c_str();
        while (*p && isspace(*p)) p++; // skip heading blanks
        if (*p == '\0' || *p == '#') continue; // skip blank line or comment line

        if (find_section(p, section, sq_section, sec_body)) continue;

        if (section.empty()) {
            if (sq_section.empty()) continue;

            // the line is data of section
            append_sq_section(sq_section, sec_body, p);
            continue;
        }

        // try to find name = value
        const char *line_head = p;
        const char *line_end = find_pair(p, section);

        // append to section
        append_sq_section(sq_section, sec_body, line_head, line_end-line_head);
    }

    if (!sq_section.empty())
        m_sections[sq_section] = sec_body;
}

void INIParser::load(const char *profile)
{
    clear();

    if (profile == NULL || profile[0] == '\0') return;
    ifstream f(profile);
    if (!f) return;

    load(f);
}

void INIParser::load(const string &profile)
{
    load(profile.c_str());
}


void INIParser::parse(const char *content)
{
    if (content == NULL)
    {
        clear();
        return;
    }

    parse(string(content));
}

void INIParser::parse(const std::string &content)
{
    clear();
    stringstream f(content, ios::in);
    load(f);
}

INIParser::INIParser(const char *profile)
{
    load(profile);
}

INIParser::INIParser(const string &profile)
{
    load(profile.c_str());
}

INIParser::~INIParser()
{
}

int INIParser::change(const string &section, const string &key, const string &val)
{
    sectionMap::iterator pos = m_ini.find(section);
    if (pos == m_ini.end()) {
        return 1;
    }
    stringMap::iterator sec_pos = pos->second.find(key);
    if (sec_pos == pos->second.end()) {
        return 2;
    }
    sec_pos->second = val;
    return 0;
}


int INIParser::change(const char *section, const char *key, const char *val)
{
    string s(section);
    string k(key);
    string v(val);

    return change(s, k, v);
}


const string &INIParser::get_string(const string &section, const string &key, const string &def)const
{
    sectionMap::const_iterator pos = m_ini.find(section);
    if (pos == m_ini.end()) {
        return def;
    }
    stringMap::const_iterator sec_pos = pos->second.find(key);
    if (sec_pos == pos->second.end()) {
        return def;
    }
    return sec_pos->second;
}

const char *INIParser::get_string(const char *section, const char *key, const char *def)const
{
    string s(section);
    string k(key);

    const string &r = get_string(s, k, m_none);
    if (r == m_none) return def;
    return r.c_str();
}


int INIParser::get_int(const string &section, const string &key, int def)const
{
    const string &s = get_string(section, key, m_none);
    if (s == m_none) {
        return def;
    }
    return atoi(s.c_str());
}

int INIParser::get_int(const char *section, const char *key, int def)const
{
    const char *i = get_string(section, key);
    if (i == NULL) return def;
    return atoi(i);
}


unsigned INIParser::get_unsigned(const string &section, const string &key, unsigned def)const
{
    const string &s = get_string(section, key, m_none);
    if (s == m_none) {
        return def;
    }
    return strtoul(s.c_str(), NULL, 10);
}

unsigned INIParser::get_unsigned(const char *section, const char *key, unsigned def)const
{
    const char *i = get_string(section, key);
    if (i == NULL) return def;
    return strtoul(i, NULL, 10);
}

bool INIParser::get_bool(const string &section, const string &key, bool def)const
{
    const string &s = get_string(section, key, m_none);
    if (s == m_none) {
        return def;
    }
    const char *cs = s.c_str();
    if (!strcasecmp(cs, "Y") ||
        !strcasecmp(cs, "YES") ||
        !strcasecmp(cs, "T") ||
        !strcasecmp(cs, "True"))
    {
        return true;
    }

    if (atoi(cs)) return true;
    return false;
}

bool INIParser::get_bool(const char *section, const char *key, bool def)const
{
    string s(section);
    string k(key);
    return get_bool(s, k, def);
}


string &INIParser::get_section(const string &section)
{
    string s;
    s.append("[");
    s.append(section);
    s.append("]");

    stringMap::iterator pos = m_sections.find(s);
    if (pos == m_sections.end()) return NOTHING;
    return pos->second;
}

const char *INIParser::get_section(const char *section)
{
    string s(section);
    string &r = get_section(s);
    if (r == NOTHING) return NULL;
    return r.c_str();
}

void INIParser::get_sections(sectionSet &sets)
{
    for (sectionMap::const_iterator iter = m_ini.begin();iter != m_ini.end();iter++)
        sets.insert(iter->first);
}
const stringMap *INIParser::get_section_map(const std::string &section)
{
    sectionMap::const_iterator pos = m_ini.find(section);
    if (pos == m_ini.end()) {
        return NULL;
    }
    return &(pos->second);
}

const stringMap *INIParser::get_section_map(const char *section)
{
    string s(section);
    return get_section_map(s);
}


void INIParser::dump()const
{
    sectionMap::const_iterator pos;
    stringMap::const_iterator p;
    cout << "=========== Dump key by key ===========" << endl;
    for (pos = m_ini.begin(); pos != m_ini.end(); pos++) {
        cout << '[' << pos->first << ']' << endl;
        for (p=pos->second.begin(); p!=pos->second.end(); p++) {
            cout << p->first << " = " << p->second << endl;
        }
        cout << endl;
    }

    cout << "=========== Dump section by section ===========" << endl;
    for (p = m_sections.begin(); p != m_sections.end(); p++) {
        cout << p->first << endl;
        cout << p->second << endl;
    }
}

}

#ifdef _TEST_
int main(int argc, char *argv[])
{
    if (argc == 1) {
        printf("usage: %s <ini>\n", argv[0]);
        return 1;
    }

    utility::INIParser ini(argv[1]);
    ini.dump();
    return 0;
}
#endif

