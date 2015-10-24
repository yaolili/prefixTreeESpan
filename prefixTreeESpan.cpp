#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <list>
#include <algorithm>
#include <time.h>

using namespace std;

struct triple
{
    int treeID;  //树的编号
    int offset;  //投影实例偏移量
    int attach;  //后面的节点连接到前缀的第attach个节点上
};

typedef vector<int> vecInt;
typedef vector<triple>  PROJECTION;
typedef vector<vector<int> > vecWTVecInt;

typedef map<int, int> MAP;
typedef map<int, vecInt> mapVecInt;
typedef map<vecInt, vecInt> mapVec;
typedef multimap<int, int> multiMAP;

typedef vector<int>::iterator iterVec;
typedef vecWTVecInt::iterator iterVecWTVecInt;
typedef map<int, int>::iterator iterMap;
typedef map<vecInt, vecInt>::iterator iterMapVecInt;
typedef multimap<int, int>::iterator iterMultiMap;


int miniSupport;
vecWTVecInt treeData, result;

int attach(const vecInt partPrefix, const int offset, const vecInt treeDataInLine)
{
    int labelNum = 0;
    MAP labelAttach;
    //initialize attachNum an illegal value: -1
    int attachNum = -1;
    iterMap iter;
    for(int i = 0; i < partPrefix.size(); i++)
    {
        if(partPrefix[i] != -1)
        {
            labelNum++;
            labelAttach[partPrefix[i]] = labelNum;
            //cout<<"add labelAttach, ["<< partPrefix[i]<<", "<<labelNum<<"]"<<endl;
        }
        else
        {
            iter = labelAttach.end();
            //wrong with: iter = labelAttach.end() - 1;
            iter--;
            //cout<<"delete labelAttach, ["<< iter->first<<", "<<iter->second<<"]"<<endl;
            labelAttach.erase(iter);
        }
    }

    /*
    //test usage!
    cout<<"labelAttach is:";
    for(iterMap iter = labelAttach.begin(); iter != labelAttach.end(); iter++)
    {
        cout<<"["<< iter->first<< ", "<< iter->second<< " ]"<<endl;
    }
    */

    //find out the attachNum
    for(int j = offset; j < treeDataInLine.size(); j++)
    {
        if(labelAttach.size())
        {
            iter = labelAttach.end();
            iter--;
            if(treeDataInLine[j] == -1)
            {
                labelAttach.erase(iter);
            }
            else
            {
                attachNum = iter->second;
            }
        }
    }
    //it's wrong when return -1
    return attachNum;
}

//find out the -1 offset matched with label
int flagOffset(const int labelOffset, const vecInt treeDataInLine)
{
    int labelCount = 0;
    int flagCount = 0;
    for(int i = labelOffset; i < treeDataInLine.size(); i++)
    {
        if(treeDataInLine[i] == -1)
            flagCount++;
        else labelCount++;
        if(labelCount == flagCount)
            return i;
    }
    //it's wrong when return -1
    return -1;
}

//to figure out whether <label, ID> exist in labelWithID
bool isSet(const int label, const int ID, multiMAP labelWithID)
{
    iterMultiMap upBound, lowBound;
    lowBound = labelWithID.lower_bound(label);
    upBound = labelWithID.upper_bound(label);
    while(lowBound != upBound)
    {
        if(lowBound->second == ID)
        {
            return true;
        }
        lowBound++;
    }
    return false;
}

vecInt growthFactor(const PROJECTION projection)
{
    MAP labelWithCount;
    multiMAP labelWithID;
    vecInt factor;
    for(int i = 0; i < projection.size(); i++)
    {
        int id = projection[i].treeID;
        int offset = projection[i].offset;
        int label = treeData[id][offset];
        //add the label's count value if <label, id> hasn't appeared before
        if(!isSet(label, id, labelWithID))
        {
            if(labelWithCount.find(label) != labelWithCount.end())
                labelWithCount[label]++;
            else labelWithCount[label] = 1;
        }
    }
    //find out the label whose count value is more than miniSupport
    for(iterMap iter = labelWithCount.begin(); iter != labelWithCount.end(); iter++)
    {
        if(iter->second >= miniSupport)
            factor.push_back(iter->first);
    }

    return factor;
}


void updateResult(const vecInt prefix, const vecWTVecInt newPrefix)
{
    for(int i = 0; i < result.size(); i++)
    {
        //find prefix in result, delete it and add newPrefix
        if(result[i] == prefix)
        {
            result.erase(result.begin() + i);
            for(int j = 0; j < newPrefix.size(); j++)
                result.push_back(newPrefix[j]);

        }
    }
}

vecWTVecInt updataPrefix(const PROJECTION projection, vecInt factor, const vecInt prefix, mapVec &prefixTreeID)
{
    /*
    //test usage!
    cout<< "in updataPrefix, prefix is: ";
    for(int i = 0; i < prefix.size(); i++)
        cout<< prefix[i]<< " ";
    cout<< "\n";
    */

    vecWTVecInt newPrefix;
    vecInt labelArray;
    mapVecInt labelNewPrefix;
    for(int i = 0; i < projection.size(); i++)
    {
        int treeID = projection[i].treeID;
        int offset = projection[i].offset;
        int attach = projection[i].attach;
        int label = treeData[treeID][offset];

        iterVec iterLabelArray = find(labelArray.begin(), labelArray.end(), label);
        //if label appeared before, only need to update prefixTreeID
        if(iterLabelArray != labelArray.end())
        {
            vecInt tmpPrefix = labelNewPrefix[label];
            vecInt tmpTreeID = prefixTreeID[tmpPrefix];
            iterVec iterID = find(tmpTreeID.begin(), tmpTreeID.end(), treeID);
            //update prefixTreeID only when the id hasn't been record
            if(iterID == tmpTreeID.end())
            {
                prefixTreeID[tmpPrefix].push_back(treeID);

                /*
                //test usage!
                cout<< "in updatePrefix, prefixTreeID: <[";
                for(int j = 0; j < tmpPrefix.size(); j++)
                    cout<< tmpPrefix[j]<< " ";
                cout<< "], [";
                for(int j = 0; j < prefixTreeID[tmpPrefix].size(); j++)
                    cout<< prefixTreeID[tmpPrefix][j]<< " ";
                cout<< "]>"<< endl;
                */
            }
        }
        else
        {
            labelArray.push_back(label);
            iterVec iter = find(factor.begin(), factor.end(), label);
            //current label is frequent
            if(iter != factor.end())
            {
                int num = 0;
                vecInt tmp;
                for(int j = 0; j < prefix.size(); j++)
                {
                    tmp.push_back(prefix[j]);
                    if(prefix[j] == -1) continue;
                    num++;
                    if(num == attach)
                    {
                        tmp.push_back(label);
                        tmp.push_back(-1);
                    }
                }
                newPrefix.push_back(tmp);
                labelNewPrefix[label] = tmp;
                prefixTreeID[tmp] = vecInt(1, treeID);
            }
        }
    }
    //after finding out all new prefixes, you need to updateResult
    updateResult(prefix, newPrefix);
    return newPrefix;
}

//get part prefix in this way:[2 1 -1 3 -1 -1] -> [2 1 -1 3]
vecInt partPrefix(const vecInt prefix)
{
    vecInt part;
    int offset;
    for(int i = prefix.size()-1; i >=0; i--)
    {
        if(prefix[i] != -1)
        {
            offset = i;
            break;
        }
    }
    for(int j = 0; j <= offset; j++)
        part.push_back(prefix[j]);
    return part;
}


PROJECTION proDB(const vecInt prefix, mapVec prefixTreeID)
{
    PROJECTION projection;
    vecInt part = partPrefix(prefix);

    /*
    //test usage!
    for(int i = 0; i < part.size(); i++)
    {
        cout<< "part["<<i<<"]:"<< part[i]<<endl;
    }
    */

    //check if prefixTreeID is empty
    if(prefixTreeID.empty())
        cout<< "PROJECTION proDB(const vecInt prefix, const mapVec prefixTreeID) "
            << "argument: prefixTreeID is empty!"<< endl;

    //for each prefix, preficTreeID has only one element
    iterMapVecInt point = prefixTreeID.begin();
    vecInt treeID = point->second;

    /*
    //test usage!
    for(int i = 0; i < treeID.size(); i++)
    {
        cout<< "treeID["<<i<<"]:"<< treeID[i]<<endl;
    }
    */

    //for each treeID, find out the projection of prefix
    for(int i = 0; i < treeID.size(); i++)
    {
        int ID = treeID[i];
        bool flag = true;

        /*
        //test usage!
        cout<< "treeData["<<ID<<"]: ";
        for(int j = 0; j < treeData[ID].size(); j++)
        {
            cout<< treeData[ID][j]<< " ";
        }
        cout<<"\n"<<endl;
        */

        iterVec first = treeData[ID].begin();
        iterVec last = treeData[ID].end();

        while(flag)
        {
            iterVec iter = search(first, last, part.begin(), part.end());
            if(iter + prefix.size() < last)
            {
                //initialize low & up bound as the last label offset in part
                int curLow = iter - treeData[ID].begin() + part.size() - 1;
                int curUp = curLow;

                //from the back forward
                for(int i = part.size() - 1; i >= 0; i--)
                {
                    int labelOffset = iter - treeData[ID].begin() + i;
                    if(treeData[ID][labelOffset] == -1)
                        continue;
                    int offset = flagOffset(labelOffset, treeData[ID]);
                    //if current section doesn't include former section, then continue
                    if(offset < curLow) continue;
                    //find out the first label in legal section & break out
                    for(int j = curUp + 1; j < offset; j++)
                    {
                        if(treeData[ID][j] == -1) continue;
                        triple node;
                        node.treeID = ID;
                        node.offset = j;
                        node.attach = attach(part, j, treeData[ID]);
                        projection.push_back(node);
                        //cout<<" projection add node, id = "<< ID<<" , offset = "<<j<<"attach = "<< node.attach<< endl;
                        break;
                    }
                    curLow = labelOffset;
                    curUp = offset > curUp ? offset : curUp;
                }
            }
            else flag = false;
            //remember to change the value of first
            first = iter + part.size();
        }
    }
    return projection;
}

void fre(PROJECTION projection, vecInt prefix)
{
    /*
    //test usage!
    cout<< "in fre, prefix is: ";
    for(int i = 0; i < prefix.size(); i++)
        cout<< prefix[i]<< " ";
    cout<< "\n";
    cout<< "in fre, projection is: "<< endl;
    for(int j = 0; j < projection.size(); j++)
    {
        cout<< "<"<< projection[j].treeID<< ", "<< projection[j].offset<< ", "<< projection[j].attach<< ">"<< endl;
    }
    */

    vecInt factor = growthFactor(projection);

    /*
    //test usage!
    cout<< "factor is: ";
    for(int i = 0; i < factor.size(); i++)
        cout<< factor[i]<< " ";
    cout<< "\n";
    */

    if(factor.empty()) return;
    mapVec prefixTreeID;
    vecWTVecInt newPrefix = updataPrefix(projection, factor, prefix, prefixTreeID);

    /*
    //test usage!
    cout<< "in fre, newPrefix is:"<< endl;
    for(int i = 0; i < newPrefix.size(); i++)
    {
        cout<< "[";
        for(int j = 0; j < newPrefix[i].size(); j++)
        {
            cout<< newPrefix[i][j]<< " ";
        }
        cout<< "]"<< endl;
    }
    */

    for(int i = 0; i < newPrefix.size(); i++)
    {
        PROJECTION newProjection = proDB(newPrefix[i], prefixTreeID);
        fre(newProjection, newPrefix[i]);
    }
    return;
}

void prefixTreeESpan(MAP labelWithCount)
{
    for(iterMap iter = labelWithCount.begin(); iter != labelWithCount.end(); iter++)
    {
        if(iter->second >= miniSupport)
        {
            vecInt prefix(1, iter->first);
            prefix.push_back(-1);
            result.push_back(prefix);

            //find out treeID of each prefix
            mapVec prefixTreeID;
            for(int i = 0; i < treeData.size(); i++)
            {
                iterVec point = find(treeData[i].begin(), treeData[i].end(), iter->first);
                if(point != treeData[i].end())
                {
                    prefixTreeID[prefix].push_back(i);
                }
            }

            /*
            //test usage!
            for(int i = 0; i < prefix.size(); i++)
            {
                cout<< "prefix["<<i<<"]:"<< prefix[i]<<endl;
            }
            cout<<"prefixTreeID: [";
            for(iterMapVecInt iter = prefixTreeID.begin(); iter != prefixTreeID.end(); iter++)
            {
                for(int j = 0; j < iter->first.size(); j++)
                {
                    cout<< iter->first[j]<< " ";
                }
                cout<<"],[";
                for(int j = 0; j < iter->second.size(); j++)
                {
                    cout<< iter->second[j]<<" ";
                }
                cout<<"]"<< endl;
            }
            */

            PROJECTION projection = proDB(prefix, prefixTreeID);
            fre(projection, prefix);

        }
    }
    return;
}


int main(int argc, char *argv[])
{
    if(argc < 4)
    {
        cout<< "argv[1]: input file path"<< endl;
        cout<< "argv[2]: mini_support"<< endl;
        cout<< "argv[3]: output file path"<< endl;
        return 0;
    }

    /*
    *line: each line of file read from filePath;
    *treeData: the whole data from file;
    *labelWithCount: labels appear in the treeData and their counts;
    *istringstream ss:string type of line should be changed into int type
    */

    clock_t start, finish;

    string line, filePath = argv[1], outPut = argv[3];
    cout<< outPut<< endl;
    miniSupport = atoi(argv[2]);
    MAP labelWithCount;
    mapVecInt labelTreeID;
    ifstream input;
    freopen(outPut.c_str(), "w", stdout);

    cin>> filePath;
    input.open(filePath.c_str());

    if(!input)
    {
        cout<< "Invalid file filePath"<< endl;
        return 0;
    }

    while(getline(input, line))
    {
        vector<int> curTree;
        int label;
        istringstream ss(line);
        while(ss >> label)
        {
            curTree.push_back(label);
            if(label != -1)
            {
                if(labelWithCount.find(label) != labelWithCount.end())
                {
                    labelWithCount[label]++;
                }
                else labelWithCount[label] = 1;
            }

        }
        /* test usage!
        for(int i = 0; i < curTree.size(); i++)
            cout<< curTree[i]<< " ";
        cout<< "\n";
        */

        treeData.push_back(curTree);
    }

    /*
    // test usage!
    for(map<int, int>::iterator iter = labelWithCount.begin(); iter != labelWithCount.end(); iter++)
    {
            cout<< "in "<< iter->first<< " : "<< iter->second<< endl;
    }
    cout<< "mini_support is:"<< miniSupport <<endl;
    */

    start = clock();
    prefixTreeESpan(labelWithCount);
    finish = clock();
    cout<<"Time used is "<< (double)(finish - start) / CLOCKS_PER_SEC<<" second."<<endl;

    //print result;
    for(int i = 0; i < result.size(); i++)
    {
        for(int j = 0; j < result[i].size(); j++)
            cout<<result[i][j]<< " ";
        cout<<"\n";
    }
    return 0;
}

