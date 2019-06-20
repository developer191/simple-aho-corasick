#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <vector>
#include <map>
#include <queue>
#include <tuple>

using namespace std;

vector<string> file2vectors(char *fpath)
{

    string str;
    vector<string> sentences;

    ifstream ifs(fpath);

    if (ifs.fail())
    {
        cerr << "Failed to open file." << endl;
        exit(1);
    }

    getline(ifs, str); // 読み飛ばし
    while (getline(ifs, str))
    {
        sentences.push_back(str);
    }
    return sentences;
}

// State Machine Class
class State
{
public:
    int id;
    map<char, int> nexts;

    bool has_key(char c)
    {
        if (nexts.count(c) < 1)
            return false;
        return true;
    }
    int next_state(char c)
    {
        return nexts[c];
    }

    void regist_nextstate(char c, int s)
    {
        nexts[c] = s;
    }

    State(int id)
    {
        this->id = id;
    }

    ~State()
    {
    }
};

class AC
{

private:
    vector<string> keywords;
    map<int, vector<string>> outputs;
    map<int, int> failure;
    vector<State> states;
    int cnt = 0;

    // Algorithm 2
    void construct_goto_func()
    {
        int idx;
        int state;
        string keyword;
        states.push_back(*(new State(0)));
        for (int i = 0; i < this->keywords.size(); i++)
        {
            keyword = this->keywords[i];
            const char *chars = keyword.c_str();
            state = 0;
            idx = 0;
            while (idx < strlen(chars) && states[state].has_key(chars[idx]))
            {
                state = states[state].nexts[chars[idx]];
                idx++;
            }
            for (int j = idx; j < strlen(chars); j++)
            {
                cnt++;
                states[state].regist_nextstate(chars[j], cnt);
                states.push_back(*(new State(cnt)));
                state = cnt;
            }
            this->outputs[cnt].push_back(keyword);
        }
    }

    // Algorithm 3
    void construct_failure_func()
    {
        queue<int> que;
        int r, a;
        int f, s;
        int q;
        int state;

        for (auto next = this->states[0].nexts.begin(); next != states[0].nexts.end(); ++next)
        {
            que.push(next->second);
            failure[next->second] = 0;
        }

        while (!que.empty())
        {
            r = que.front();
            que.pop();
            for (auto next = states[r].nexts.begin(); next != states[r].nexts.end(); ++next)
            {
                a = next->first;
                s = next->second;

                que.push(s);
                if (r > 0)
                {
                    state = failure[r];

                    while (g(state, a) == -1)
                    {
                        state = failure[state];
                    }
                    this->failure[s] = g(state, a);
                    auto words = this->outputs[this->failure[s]];
                    if (!words.empty())
                    {
                        this->outputs[s].insert(this->outputs[s].end(), words.begin(), words.end());
                    }
                }
            }
        }
    }

    // g(state, a) in the paper.
    int g(int state, char a)
    {
        if (this->states[state].has_key(a))
        {
            return this->states[state].nexts[a];
        }
        else if (!this->states[0].has_key(a))
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }

public:
    AC(vector<string> keywords)
    {
        this->keywords = keywords;
        construct_goto_func();
        construct_failure_func();
    }

    // Algorithm 1
    tuple<string, int> match(string text_string)
    {
        map<int, vector<int>> match_places;
        int state = 0;
        int length = 0;
        int dis = 0;
        int bold_num = 0; // count matched words
        int col_len;
        unsigned long long int search_cnt = 0;
        string contents = text_string;
        const char *chars = text_string.c_str();

        // string match
        for (int i = 0; i < text_string.length(); i++)
        {
            search_cnt++;
            while (g(state, chars[i]) == -1)
            {
                state = failure[state];
                search_cnt++;
            }
            state = g(state, chars[i]);
            if (!outputs[state].empty())
            {
                col_len = 0;
                for (int j = 0; j < outputs[state].size(); j++)
                {
                    col_len = (outputs[state][j].length() > col_len) ? outputs[state][j].length() : col_len;
                }
                match_places[i - col_len + 1].push_back(i);
            }
        }

        // make contents formated html
        for (auto match = match_places.begin(); match != match_places.end(); ++match)
        {
            dis = match->second[0];
            if (match->second.size() > 1)
            {
                for (int i = 0; i < match->second.size(); i++)
                {
                    dis = (match->second[i] > dis) ? match->second[i] : dis;
                }
            }
            contents.insert(match->first + bold_num * 7, "<b>");
            contents.insert(dis + bold_num * 7 + 4, "</b>");
            bold_num++;
        }
        return {contents, search_cnt};
    }
};

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        cerr << "exec format: `./aho-corasick documentfile keyword[1] keyword[2], ..., keyword[n]`" << endl;
        exit(1);
    }

    string sentences;
    auto sentences_vector = file2vectors(argv[1]);
    for (int i = 0; i < sentences_vector.size(); i++)
    {
        sentences += sentences_vector[i] + "\n";
    }
    vector<string> keywords;
    for (int i = 2; i < argc; i++)
    {
        keywords.push_back(argv[i]);
    }

    AC ac(keywords);
    auto [contents, cnt] = ac.match(sentences);
    cerr << "Comparing Numbers: " << cnt << endl;
    cout << "<!DOCTYPE>"
            "<html>"
            "<head>"
            "<title> Aho - Corasick sample</title>"
            "</head>"
            "<body> "
         << contents
         << "</body>"
            "</html>"
         << endl;
}
