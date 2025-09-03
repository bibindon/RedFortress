#include <iostream>
#include "HeaderOnlyCsv.hpp"
#include <algorithm>
#include <cmath>

//---------------------------------------------------------------------------------
// prolitan.xを読み込み、木や石の配置データと敵の配置データをcsvで作成する。
//
// - 頂点データには何度も同じ点が出現する。
//   点は3つ以上のポリゴンで使われているので３回以上登場することになる。
//   例えば、正四面体はひとつの頂点が３つの三角形から共有されている。
//   これらの重複を排除する
//
// - 頂点にランダムで木や石、敵を割り当てる。同一の頂点に二つ以上割り当てられないようにする
//
// - 木や石、敵が割り当てられる確率を設定できるようにする
//
// - すべての頂点にどれかを割り当てる必要はない。
//   100％だったら全部の頂点に何かしらは割り当てる、
//   50％だったら半分の頂点に何かしらを割り当てる、という確立を設定できるようにする
//
// - 特定の位置ではこれが配置されてほしい、というのがあるのでそれをくみ取れるようにする。
//   例えば、バナナがたくさん取れるエリア、や特定のモンスターが現れるエリアなど。
// 
// - 同じ種類のモンスターや同じ種類のオブジェクトがまとまって配置されるようにする
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
// 座標構造体
//---------------------------------------------------------------------------------
struct stPos;

//---------------------------------------------------------------------------------
// 分割関数
//---------------------------------------------------------------------------------
std::vector<std::string> split(const std::string& input, char delimiter);

// 2点間の距離
float distance(const float& x1, const float& z1, const float& x2, const float& z2);

//---------------------------------------------------------------------------------
// 読み込み関数
//---------------------------------------------------------------------------------
std::vector<stPos> ReadX(const std::string& xPath);

struct stPos
{
    stPos()
        : m_x(0.f)
        , m_y(0.f)
        , m_z(0.f)
    {

    }

    stPos(const float _x, const float _y, const float _z)
        : m_x(_x)
        , m_y(_y)
        , m_z(_z)
    {

    }

    float m_x;
    float m_y;
    float m_z;

    // 比較演算子（ソート用）
    bool operator<(const stPos& other) const
    {
        if (m_x != other.m_x)
        {
            return m_x < other.m_x;
        }

        if (m_y != other.m_y)
        {
            return m_y < other.m_y;
        }

        return m_z < other.m_z;
    }

    // 同値判定（重複判定用）
    bool operator==(const stPos& other) const
    {
        if ((other.m_x - 0.1f) < m_x && m_x < (other.m_x + 0.1f))
        {
            if ((other.m_y - 0.1f) < m_y && m_y < (other.m_y + 0.1f))
            {
                if ((other.m_z - 0.1f) < m_z && m_z < (other.m_z + 0.1f))
                {
                    return true;
                }
            }
        }

        return false;
    }
};


// マップオブジェクト情報
struct stMapObj
{
    stPos m_pos;

    int m_id = 0;

    // 表示倍率
    int m_scale = 1;
};

// 敵情報
struct stEnemyInfo
{
    stPos m_pos;

    // ここではstd::wstringではなくstd::stringを使う。
    // csvファイルに出力するため
    std::string m_id;
    int m_HP = 100;
};

//---------------------------------------------------------------------------------
// 割り当て関数
//---------------------------------------------------------------------------------
void AssignContent(const std::vector<stPos>& posList,
                   std::vector<stMapObj>* mapObjList,
                   std::vector<stEnemyInfo>* enemyList);

void WriteToCsv(const std::vector<stMapObj>& mapObjList, const std::vector<stEnemyInfo>& enemyList);

//---------------------------------------------------------------------------------
// main関数
//---------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        MessageBox(NULL, L"xファイルを一つ渡してください", L"エラー", MB_OK);
        return -1;
    }

    auto posList = ReadX(argv[1]);

    std::vector<stMapObj> mapObjList;
    std::vector<stEnemyInfo> enemyList;

    AssignContent(posList, &mapObjList, &enemyList);

    WriteToCsv(mapObjList, enemyList);

    return 0;
}

std::vector<std::string> split(const std::string& input, char delimiter)
{
    std::vector<std::string> result;
    std::string current;

    for (char ch : input)
    {
        if (ch == delimiter)
        {
            result.push_back(current);
            current.clear();
        }
        else
        {
            current += ch;
        }
    }

    // 最後の要素も追加
    result.push_back(current);

    return result;
}

float distance(const float& x1, const float& z1, const float& x2, const float& z2)
{
    float dx = x2 - x1;
    float dz = z2 - z1;
    return std::sqrt(dx * dx + dz * dz);
}

std::vector<stPos> ReadX(const std::string& xPath)
{
    std::ifstream ifs(xPath);
    if (!ifs.is_open())
    {
        throw std::exception("Failed to open x file.");
    }

    int startIndex = 0;
    std::vector<std::string> vertexLine;

    {
        std::string line;
        bool vertexStart = false;
        while (std::getline(ifs, line))
        {
            // "    Mesh { // Grid mesh"の次の次の行からが頂点データ。
            if (line == "    Mesh { // Grid mesh")
            {
                std::getline(ifs, line);
                std::getline(ifs, line);
                startIndex++;
                startIndex++;
                vertexStart = true;
                continue;
            }

            // 「,,」が登場したら頂点データ終了
            if (vertexStart && line.find(";;") != std::string::npos)
            {
                vertexLine.push_back(line);
                break;
            }

            if (vertexStart)
            {
                vertexLine.push_back(line);
            }
            else
            {
                startIndex++;
            }
        }
    }

    //----------------------------------------------------------
    // 頂点データは以下のようになっている
    // 
    // 72.274902;1602.057251;-0.613953;,
    // 72.274902;1602.057251;-0.613953;,
    // 141.719360;1602.057251;-0.613953;,
    // 141.719360;1671.501709;-0.613953;,
    // 141.719360;1671.501709;-0.613953;,
    //
    // 末尾の文字を削除してからセミコロンで分割すればよい。
    //----------------------------------------------------------

    std::vector<stPos> posList;

    for (size_t i = 0; i < vertexLine.size(); ++i)
    {
        std::string line = vertexLine.at(i);
        line.pop_back();
        auto vs = split(line, ';');
        stPos _stPos { };
        _stPos.m_x = std::stof(vs.at(0));
        _stPos.m_y = std::stof(vs.at(2));
        _stPos.m_z = std::stof(vs.at(1));
        posList.push_back(_stPos);
    }

    // 重複排除
    std::sort(posList.begin(), posList.end());
    posList.erase(std::unique(posList.begin(), posList.end()), posList.end());

    return posList;
}

struct stMapDef
{
    int m_id = 0;

    // このエリアには必ず現れてほしい、という位置情報
    std::vector<stPos> m_mustAreaList;

    // どこにでも現れるオブジェクトか否か
    bool m_bCommon = true;

    // 出現確率 0.0f ~ 100.0f 小数点以下1桁まで
    float m_rate = 0.f;

    // 表示倍率
    int m_scale = 1;
};

struct stEnemyDef
{
    std::string m_id;

    // このエリアには必ず現れてほしい、という位置情報
    std::vector<stPos> m_mustAreaList;

    // どこにでも現れる敵か否か
    bool m_bCommon = true;

    // 出現確率 0.0f ~ 100.0f 小数点以下1桁まで
    float m_rate = 0.f;

    int m_HP = 100;
};

void AssignContent(const std::vector<stPos>& posList,
                   std::vector<stMapObj>* mapObjList,
                   std::vector<stEnemyInfo>* enemyList)
{
    // 「このエリアにはこのモンスター」・・・というのがある。
    // 同様に「このエリアにはこの植物」・・・というのがある。
    // 
    // そのエリアの中心座標を登録しておく。
    // その中心座標から100m以内だったら該当する、という風にする
    // 
    // 一方で、どこでも共通して配置されてほしいものもある。
    // CommonとUniqueという区分を持たせる
    //
    // 70％の確率でCommonの中から一つ、30％の確率でUniqueから1つというようにする？

    std::vector<stMapDef> mapInfoList;

    //---------------------------------------------------------
    // 木
    //---------------------------------------------------------
    {
        stMapDef obj;
        obj.m_id = 1;
        obj.m_bCommon = true;
        obj.m_rate = 30.f;
        mapInfoList.push_back(obj);
    }

    //---------------------------------------------------------
    // 草
    //---------------------------------------------------------
    {
        stMapDef obj;
        obj.m_id = 4;
        obj.m_bCommon = true;
        obj.m_rate = 24.f;
        mapInfoList.push_back(obj);
    }

    //---------------------------------------------------------
    // 草2
    //---------------------------------------------------------
    {
        stMapDef obj;
        obj.m_id = 5;
        obj.m_bCommon = true;
        obj.m_rate = 15.f;
        mapInfoList.push_back(obj);
    }

    //---------------------------------------------------------
    // 岩
    //---------------------------------------------------------
    {
        stMapDef obj;
        obj.m_id = 6;
        obj.m_bCommon = true;
        obj.m_rate = 1.f;
        obj.m_scale = 5;
        mapInfoList.push_back(obj);
    }

    //---------------------------------------------------------
    // どんぐり
    //---------------------------------------------------------
    {
        stMapDef obj;
        obj.m_id = 7;
        obj.m_bCommon = true;
        obj.m_rate = 8.f;
        mapInfoList.push_back(obj);
    }

    //---------------------------------------------------------
    // 沢山の草
    //---------------------------------------------------------
    {
        stMapDef obj;
        obj.m_id = 8;
        obj.m_bCommon = true;
        obj.m_rate = 1.f;
        mapInfoList.push_back(obj);
    }

    //---------------------------------------------------------
    // ソテツ
    //---------------------------------------------------------
    {
        stMapDef obj;
        obj.m_id = 10;
        obj.m_bCommon = false;
        obj.m_rate = 0.f;

        // この辺り一帯にソテツを植えまくる
        stPos _stPos { -1727.95f, 201.72f, -1231.66f };
        obj.m_mustAreaList.push_back(_stPos);
        mapInfoList.push_back(obj);
    }

    //---------------------------------------------------------
    // 砂利
    //---------------------------------------------------------
    {
        stMapDef obj;
        obj.m_id = 11;
        obj.m_bCommon = true;
        obj.m_rate = 4.f;
        mapInfoList.push_back(obj);
    }

    //---------------------------------------------------------
    // 大きな木
    //---------------------------------------------------------
    {
        stMapDef obj;
        obj.m_id = 12;
        obj.m_bCommon = true;
        obj.m_rate = 4.f;
        obj.m_scale = 5;
        mapInfoList.push_back(obj);
    }

    //---------------------------------------------------------
    // 細い木
    //---------------------------------------------------------
    {
        stMapDef obj;
        obj.m_id = 13;
        obj.m_bCommon = true;
        obj.m_rate = 13.f;
        obj.m_scale = 1;
        mapInfoList.push_back(obj);
    }

    std::vector<stEnemyDef> enemyInfoList;

    //---------------------------------------------------------
    // リッポウタイ
    //---------------------------------------------------------
    {
        stEnemyDef enemy;
        enemy.m_id = "cube";
        enemy.m_bCommon = true;
        enemy.m_rate = 10.f;

        stPos _stPos;
        _stPos = stPos { -285, 14, 540 };
        enemy.m_mustAreaList.push_back(_stPos);

        _stPos = stPos { -160, 56, 530 };
        enemy.m_mustAreaList.push_back(_stPos);

        enemyInfoList.push_back(enemy);
    }

    //---------------------------------------------------------
    // キュウ
    //---------------------------------------------------------
    {
        stEnemyDef enemy;
        enemy.m_id = "sphere";
        enemy.m_bCommon = true;
        enemy.m_rate = 10.f;
        enemyInfoList.push_back(enemy);
    }

    //---------------------------------------------------------
    // エンバン
    //---------------------------------------------------------
    {
        stEnemyDef enemy;
        enemy.m_id = "enban";
        enemy.m_bCommon = true;
        enemy.m_rate = 10.f;
        enemyInfoList.push_back(enemy);
    }
    
    //---------------------------------------------------------
    // エンチュウ
    //---------------------------------------------------------
    {
        stEnemyDef enemy;
        enemy.m_id = "enchu";
        enemy.m_bCommon = true;
        enemy.m_rate = 10.f;
        enemyInfoList.push_back(enemy);
    }

    //---------------------------------------------------------
    // ビッグリッポウタイ
    //---------------------------------------------------------
    {
        stEnemyDef enemy;
        enemy.m_id = "bigCube";
        enemy.m_bCommon = true;
        enemy.m_rate = 10.f;
        enemyInfoList.push_back(enemy);
    }

    //---------------------------------------------------------
    // スモールリッポウタイ
    //---------------------------------------------------------
    {
        stEnemyDef enemy;
        enemy.m_id = "smallCube";
        enemy.m_bCommon = true;
        enemy.m_rate = 10.f;
        enemyInfoList.push_back(enemy);
    }

    //---------------------------------------------------------
    // ハンキュウ
    //---------------------------------------------------------
    {
        stEnemyDef enemy;
        enemy.m_id = "hankyu";
        enemy.m_bCommon = true;
        enemy.m_rate = 10.f;
        enemyInfoList.push_back(enemy);
    }

    //---------------------------------------------------------
    // ハンエン
    //---------------------------------------------------------
    {
        stEnemyDef enemy;
        enemy.m_id = "hanen";
        enemy.m_bCommon = true;
        enemy.m_rate = 10.f;
        enemyInfoList.push_back(enemy);
    }

    //---------------------------------------------------------
    // オレンジリッポウタイ
    //---------------------------------------------------------
    {
        stEnemyDef enemy;
        enemy.m_id = "orangeCube";
        enemy.m_bCommon = true;
        enemy.m_rate = 10.f;
        enemyInfoList.push_back(enemy);
    }

    //---------------------------------------------------------
    // 島民の霊
    //---------------------------------------------------------
    {
        stEnemyDef enemy;
        enemy.m_id = "ghost";
        enemy.m_bCommon = true;
        enemy.m_rate = 10.f;
        enemyInfoList.push_back(enemy);
    }

    //---------------------------------------------------------
    // マップデータには、「最初の60個はこれが出力されてほしい」、というのがあるのでそれを設定する。
    //---------------------------------------------------------
    {
        // 1
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 2;
            _stMapObj.m_pos = stPos(-270, 18.5, 523);
            _stMapObj.m_scale = 2;

            mapObjList->push_back(_stMapObj);
        }

        // 2
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 2;
            _stMapObj.m_pos = stPos(-268, 19, 504);
            _stMapObj.m_scale = 2;

            mapObjList->push_back(_stMapObj);
        }

        // 3
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 2;
            _stMapObj.m_pos = stPos(-248, 23.5, 544);
            _stMapObj.m_scale = 2;

            mapObjList->push_back(_stMapObj);
        }

        // 4
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 2;
            _stMapObj.m_pos = stPos(-225, 26, 615);
            _stMapObj.m_scale = 2;

            mapObjList->push_back(_stMapObj);
        }

        // 5
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 2;
            _stMapObj.m_pos = stPos(-230, 26, 590);
            _stMapObj.m_scale = 2;

            mapObjList->push_back(_stMapObj);
        }

        // 6
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 2;
            _stMapObj.m_pos = stPos(-266, 19, 556);
            _stMapObj.m_scale = 2;

            mapObjList->push_back(_stMapObj);
        }

        // 7
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 3;
            _stMapObj.m_pos = stPos(-270, 18.5, 523);
            _stMapObj.m_scale = 2;

            mapObjList->push_back(_stMapObj);
        }

        // 8
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 3;
            _stMapObj.m_pos = stPos(-268, 19, 504);
            _stMapObj.m_scale = 2;

            mapObjList->push_back(_stMapObj);
        }

        // 9
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 3;
            _stMapObj.m_pos = stPos(-248, 23.5, 544);
            _stMapObj.m_scale = 2;

            mapObjList->push_back(_stMapObj);
        }

        // 10
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 3;
            _stMapObj.m_pos = stPos(-225, 26, 615);
            _stMapObj.m_scale = 2;

            mapObjList->push_back(_stMapObj);
        }

        // 11
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 3;
            _stMapObj.m_pos = stPos(-230, 26, 590);
            _stMapObj.m_scale = 2;

            mapObjList->push_back(_stMapObj);
        }

        // 12
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 3;
            _stMapObj.m_pos = stPos(-266, 19, 556);
            _stMapObj.m_scale = 2;

            mapObjList->push_back(_stMapObj);
        }

        // 13 ~ 23 pass

//        // 13
//        {
//            stMapObj _stMapObj;
//            _stMapObj.m_id = 7;
//            _stMapObj.m_pos = stPos(-214, -33, 545);
//            _stMapObj.m_scale = 2;
//
//            mapObjList->push_back(_stMapObj);
//        }
//
//        // 14
//        {
//            stMapObj _stMapObj;
//            _stMapObj.m_id = 7;
//            _stMapObj.m_pos = stPos(-220, -31, 553);
//            _stMapObj.m_scale = 2;
//
//            mapObjList->push_back(_stMapObj);
//        }
//
//        // 15
//        {
//            stMapObj _stMapObj;
//            _stMapObj.m_id = 7;
//            _stMapObj.m_pos = stPos(-197, -37, 569);
//            _stMapObj.m_scale = 2;
//
//            mapObjList->push_back(_stMapObj);
//        }
//
//        // 16
//        {
//            stMapObj _stMapObj;
//            _stMapObj.m_id = 8;
//            _stMapObj.m_pos = stPos(-220, -29, 574);
//            _stMapObj.m_scale = 2;
//
//            mapObjList->push_back(_stMapObj);
//        }
//
//        // 17
//        {
//            stMapObj _stMapObj;
//            _stMapObj.m_id = 8;
//            _stMapObj.m_pos = stPos(-228, -28, 552);
//            _stMapObj.m_scale = 2;
//
//            mapObjList->push_back(_stMapObj);
//        }
//
//        // 18
//        {
//            stMapObj _stMapObj;
//            _stMapObj.m_id = 8;
//            _stMapObj.m_pos = stPos(-222, -31, 536);
//            _stMapObj.m_scale = 2;
//
//            mapObjList->push_back(_stMapObj);
//        }
//
//        // 19
//        {
//            stMapObj _stMapObj;
//            _stMapObj.m_id = 8;
//            _stMapObj.m_pos = stPos(-227, -31, 519);
//            _stMapObj.m_scale = 2;
//
//            mapObjList->push_back(_stMapObj);
//        }
//
//        // 20
//        {
//            stMapObj _stMapObj;
//            _stMapObj.m_id = 8;
//            _stMapObj.m_pos = stPos(-230, -33, 478);
//            _stMapObj.m_scale = 2;
//
//            mapObjList->push_back(_stMapObj);
//        }
//
//        // 21
//        {
//            stMapObj _stMapObj;
//            _stMapObj.m_id = 8;
//            _stMapObj.m_pos = stPos(-220, -34, 503);
//            _stMapObj.m_scale = 2;
//
//            mapObjList->push_back(_stMapObj);
//        }
//
//        // 22
//        {
//            stMapObj _stMapObj;
//            _stMapObj.m_id = 8;
//            _stMapObj.m_pos = stPos(-214, -34, 522);
//            _stMapObj.m_scale = 2;
//
//            mapObjList->push_back(_stMapObj);
//        }
//
//        // 23
//        {
//            stMapObj _stMapObj;
//            _stMapObj.m_id = 8;
//            _stMapObj.m_pos = stPos(-216, -33, 548);
//            _stMapObj.m_scale = 2;
//
//            mapObjList->push_back(_stMapObj);
//        }

        // 24, 25, 26, 27 Pass

        // 28
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 6;
            _stMapObj.m_pos = stPos(-420, 1, 415);
            _stMapObj.m_scale = 10;

            mapObjList->push_back(_stMapObj);
        }

        // 29
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 6;
            _stMapObj.m_pos = stPos(-425, 1, 410);
            _stMapObj.m_scale = 10;

            mapObjList->push_back(_stMapObj);
        }

        // 30
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 6;
            _stMapObj.m_pos = stPos(-430, 1, 405);
            _stMapObj.m_scale = 10;

            mapObjList->push_back(_stMapObj);
        }

        // yashi pass

        // 31
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 9;
            _stMapObj.m_pos = stPos(-1344, 14, -594);
            _stMapObj.m_scale = 1;

            mapObjList->push_back(_stMapObj);
        }

        // 32
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 9;
            _stMapObj.m_pos = stPos(-1344, 14, -584);
            _stMapObj.m_scale = 1;

            mapObjList->push_back(_stMapObj);
        }

        // 33
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 9;
            _stMapObj.m_pos = stPos(-1344, 14, -574);
            _stMapObj.m_scale = 1;

            mapObjList->push_back(_stMapObj);
        }

        // 34
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 9;
            _stMapObj.m_pos = stPos(-1344, 14, -564);
            _stMapObj.m_scale = 1;

            mapObjList->push_back(_stMapObj);
        }

        // 35
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 9;
            _stMapObj.m_pos = stPos(-1344, 14, -554);
            _stMapObj.m_scale = 1;

            mapObjList->push_back(_stMapObj);
        }

        // 36 passed

        // 37
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 9;
            _stMapObj.m_pos = stPos(-1354, 14, -594);
            _stMapObj.m_scale = 1;

            mapObjList->push_back(_stMapObj);
        }

        // 38
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 9;
            _stMapObj.m_pos = stPos(-1354, 14, -584);
            _stMapObj.m_scale = 1;

            mapObjList->push_back(_stMapObj);
        }

        // 39
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 9;
            _stMapObj.m_pos = stPos(-1354, 14, -574);
            _stMapObj.m_scale = 1;

            mapObjList->push_back(_stMapObj);
        }

        // 40
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 9;
            _stMapObj.m_pos = stPos(-1354, 14, -564);
            _stMapObj.m_scale = 1;

            mapObjList->push_back(_stMapObj);
        }

        // 41
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 9;
            _stMapObj.m_pos = stPos(-1354, 14, -554);
            _stMapObj.m_scale = 1;

            mapObjList->push_back(_stMapObj);
        }

        // 42 passed

        // 43
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 9;
            _stMapObj.m_pos = stPos(-1364, 14, -594);
            _stMapObj.m_scale = 1;

            mapObjList->push_back(_stMapObj);
        }

        // 44
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 9;
            _stMapObj.m_pos = stPos(-1364, 14, -584);
            _stMapObj.m_scale = 1;

            mapObjList->push_back(_stMapObj);
        }

        // 45
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 9;
            _stMapObj.m_pos = stPos(-1364, 14, -574);
            _stMapObj.m_scale = 1;

            mapObjList->push_back(_stMapObj);
        }

        // 46
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 9;
            _stMapObj.m_pos = stPos(-1364, 14, -564);
            _stMapObj.m_scale = 1;

            mapObjList->push_back(_stMapObj);
        }

        // 47
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 9;
            _stMapObj.m_pos = stPos(-1364, 14, -554);
            _stMapObj.m_scale = 1;

            mapObjList->push_back(_stMapObj);
        }

        // 48 passed

        // 49
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 9;
            _stMapObj.m_pos = stPos(-1374, 14, -594);
            _stMapObj.m_scale = 1;

            mapObjList->push_back(_stMapObj);
        }

        // 50
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 9;
            _stMapObj.m_pos = stPos(-1374, 14, -584);
            _stMapObj.m_scale = 1;

            mapObjList->push_back(_stMapObj);
        }

        // 51
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 9;
            _stMapObj.m_pos = stPos(-1374, 14, -574);
            _stMapObj.m_scale = 1;

            mapObjList->push_back(_stMapObj);
        }

        // 52
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 9;
            _stMapObj.m_pos = stPos(-1374, 14, -564);
            _stMapObj.m_scale = 1;

            mapObjList->push_back(_stMapObj);
        }

        // 53
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 9;
            _stMapObj.m_pos = stPos(-1374, 14, -554);
            _stMapObj.m_scale = 1;

            mapObjList->push_back(_stMapObj);
        }

        // 54 passed

        // 55
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 9;
            _stMapObj.m_pos = stPos(-1384, 14, -594);
            _stMapObj.m_scale = 1;

            mapObjList->push_back(_stMapObj);
        }

        // 56
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 9;
            _stMapObj.m_pos = stPos(-1384, 14, -584);
            _stMapObj.m_scale = 1;

            mapObjList->push_back(_stMapObj);
        }

        // 57
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 9;
            _stMapObj.m_pos = stPos(-1384, 14, -574);
            _stMapObj.m_scale = 1;

            mapObjList->push_back(_stMapObj);
        }

        // 58
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 9;
            _stMapObj.m_pos = stPos(-1384, 14, -564);
            _stMapObj.m_scale = 1;

            mapObjList->push_back(_stMapObj);
        }

        // 59
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 9;
            _stMapObj.m_pos = stPos(-1384, 14, -554);
            _stMapObj.m_scale = 1;

            mapObjList->push_back(_stMapObj);
        }

        // 60
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 6;
            _stMapObj.m_pos = stPos(-207, 87, 202);
            _stMapObj.m_scale = 10;

            mapObjList->push_back(_stMapObj);
        }

        // 61
        {
            stMapObj _stMapObj;
            _stMapObj.m_id = 10;
            _stMapObj.m_pos = stPos(-1727.95f, 201.72f, -1231.66f);
            _stMapObj.m_scale = 1;

            mapObjList->push_back(_stMapObj);
        }
    }

    // 固定で配置したい敵情報
    {
        {
            stEnemyInfo _stEnemyInfo;
            _stEnemyInfo.m_id = "ghost";
            _stEnemyInfo.m_pos = stPos(223, 226, -1399);
            _stEnemyInfo.m_HP = 40;

            enemyList->push_back(_stEnemyInfo);
        }
        {
            stEnemyInfo _stEnemyInfo;
            _stEnemyInfo.m_id = "ghost";
            _stEnemyInfo.m_pos = stPos(509, 19, -1263);
            _stEnemyInfo.m_HP = 40;

            enemyList->push_back(_stEnemyInfo);
        }
        {
            stEnemyInfo _stEnemyInfo;
            _stEnemyInfo.m_id = "ghost";
            _stEnemyInfo.m_pos = stPos(1112, 210, -1063);
            _stEnemyInfo.m_HP = 40;

            enemyList->push_back(_stEnemyInfo);
        }
    }

    srand((unsigned int)time(NULL));

    for (size_t i = 0; i < posList.size(); ++i)
    {
        auto& pos = posList.at(i);
        auto _rand1 = rand() % 100; // 0 ~ 99
        auto _rand2 = rand() % 1000; // 0 ~ 999

        // 90%の確率でCommonから1つ、10%の確率でUniqueから1つを選ぶ
        // 共通オブジェクト
        if (_rand1 < 90)
        {
            for (size_t j = 0; j < mapInfoList.size(); ++j)
            {
                auto& mapInfo = mapInfoList.at(j);
                if (mapInfo.m_bCommon)
                {
                    int work = (int)(mapInfo.m_rate * 10);
                    _rand2 -= work;
                
                    // 確率を引いていき、0以下になったら当選
                    if (_rand2 < 0)
                    {
                        stMapObj _stMapObj;
                        _stMapObj.m_id = mapInfo.m_id;
                        _stMapObj.m_pos = pos;
                        _stMapObj.m_scale = mapInfo.m_scale;

                        mapObjList->push_back(_stMapObj);

                        break;
                    }
                }
            }

            continue;
        }
        // ユニークオブジェクト
        else
        {
            //--------------------------------------------------------------
            // この位置の近くだったら必ずこれが表示されてほしい、というのがある。
            // それを探す。
            // 複数該当する場合がある。その場合は近いものを選択する。
            //--------------------------------------------------------------
            struct stCandidate
            {
                // マップオブジェクトか敵情報か
                bool m_isMapObj = true;

                int m_id = 0;

                std::string m_enemyId;

                float m_distance = 0.f;
            };

            std::vector<stCandidate> candidateList;

            // 100メートル以内のものをリストアップ
            for (size_t j = 0; j < mapInfoList.size(); ++j)
            {
                auto& mapInfo = mapInfoList.at(j);
                if (!mapInfo.m_bCommon)
                {
                    for (size_t k = 0; k < mapInfo.m_mustAreaList.size(); ++k)
                    {
                        float distance_ = distance(pos.m_x,
                                                   pos.m_z,
                                                   mapInfo.m_mustAreaList.at(k).m_x,
                                                   mapInfo.m_mustAreaList.at(k).m_z);
                        if (distance_ < 100.f)
                        {
                            stCandidate candidate;
                            candidate.m_isMapObj = true;
                            candidate.m_id = mapInfo.m_id;
                            candidate.m_distance = distance_;

                            candidateList.push_back(candidate);
                        }
                    }
                }
            }

            // 100メートル以内のものをリストアップ
            for (size_t j = 0; j < enemyInfoList.size(); ++j)
            {
                auto& enemyInfo = enemyInfoList.at(j);
                for (size_t k = 0; k < enemyInfo.m_mustAreaList.size(); ++k)
                {
                    float distance_ = distance(pos.m_x,
                                               pos.m_z,
                                               enemyInfo.m_mustAreaList.at(k).m_x,
                                               enemyInfo.m_mustAreaList.at(k).m_z);
                    if (distance_ < 100.f)
                    {
                        stCandidate candidate;
                        candidate.m_isMapObj = false;
                        candidate.m_enemyId = enemyInfo.m_id;
                        candidate.m_distance = distance_;

                        candidateList.push_back(candidate);
                    }
                }
            }

            // 100メートル以内のものがあれば一番近いものを登録
            if (!candidateList.empty())
            {
                auto it = std::min_element(candidateList.begin(), candidateList.end(),
                                           [](const stCandidate& a, const stCandidate& b)
                                           {
                                               return a.m_distance < b.m_distance;
                                           });
                if (it != candidateList.end())
                {
                    if (it->m_isMapObj)
                    {
                        auto it2 = std::find_if(mapInfoList.begin(), mapInfoList.end(),
                                                [&](const stMapDef& mapDef)
                                                {
                                                    return mapDef.m_id == it->m_id;
                                                });

                        stMapObj _stMapObj;
                        _stMapObj.m_id = it->m_id;
                        _stMapObj.m_pos = pos;
                        _stMapObj.m_scale = it2->m_scale;

                        mapObjList->push_back(_stMapObj);
                    }
                    else
                    {
                        auto it2 = std::find_if(enemyInfoList.begin(), enemyInfoList.end(),
                                                [&](const stEnemyDef& enemyDef)
                                                {
                                                    return enemyDef.m_id == it->m_enemyId;
                                                });

                        stEnemyInfo _stEnemyInfo;
                        _stEnemyInfo.m_id = it->m_enemyId;
                        _stEnemyInfo.m_pos = pos;
                        _stEnemyInfo.m_HP = it2->m_HP;

                        enemyList->push_back(_stEnemyInfo);
                    }

                    continue;
                }
            }

            //--------------------------------------------------------------
            // 敵は同じ種類が固まって出現してほしいので
            // 座標の3桁目の数値を使って決める
            // x = 1234, z = 567だったら、12と5を足して17とする。
            // 敵は10種類なので17を10で割って7、とすれば
            // 100メートルごとに違う種類の敵が現れることになる。
            //--------------------------------------------------------------
            {
                int work_i = 0;
                work_i += (int)pos.m_x / 100;
                work_i += (int)pos.m_z / 100;
                work_i = std::abs(work_i);
                work_i = work_i % 10;

                std::string enemyId;
                if (work_i== 0) { enemyId = "cube"; }
                else if (work_i== 1) { enemyId = "sphere"; }
                else if (work_i== 2) { enemyId = "enban"; }
                else if (work_i== 3) { enemyId = "enchu"; }
                else if (work_i== 4) { enemyId = "bigCube"; }
                else if (work_i== 5) { enemyId = "smallCube"; }
                else if (work_i== 6) { enemyId = "hanen"; }
                else if (work_i== 7) { enemyId = "hankyu"; }
                else if (work_i== 8) { enemyId = "orangeCube"; }
                else if (work_i== 9) { enemyId = "ghost"; }


                auto it = std::find_if(enemyInfoList.begin(), enemyInfoList.end(),
                                       [&](const stEnemyDef& enemyDef)
                                       {
                                           return enemyDef.m_id == enemyId;
                                       });

                stEnemyInfo enemyInfo;
                enemyInfo.m_id = enemyId;
                enemyInfo.m_pos = pos;
                enemyInfo.m_HP = it->m_HP;
                enemyList->push_back(enemyInfo);
            }
        }
    }
}

void WriteToCsv(const std::vector<stMapObj>& mapObjList, const std::vector<stEnemyInfo>& enemyList)
{
    {
        std::vector<std::vector<std::string>> vvs;
        std::vector<std::string> vs;
        vs.push_back("ID");
        vs.push_back(u8"モデルID");
        vs.push_back("X");
        vs.push_back("Y");
        vs.push_back("Z");
        vs.push_back("RotY");
        vs.push_back("Scale");
        vs.push_back(u8"表示");
        vvs.push_back(vs);
        vs.clear();

        for (size_t i = 0; i < mapObjList.size(); ++i)
        {
            vs.clear();
            auto& mapObj = mapObjList.at(i);

            vs.push_back(std::to_string(i + 1));
            vs.push_back(std::to_string(mapObj.m_id));
            vs.push_back(std::to_string(mapObj.m_pos.m_x));
            vs.push_back(std::to_string(mapObj.m_pos.m_y));
            vs.push_back(std::to_string(mapObj.m_pos.m_z));
            vs.push_back("0");
            vs.push_back(std::to_string(mapObj.m_scale));
            vs.push_back("y");
            vvs.push_back(vs);
        }

        csv::Write("map_obj.csv", vvs);
    }

    {
        std::vector<std::vector<std::string>> vvs;
        std::vector<std::string> vs;
        vs.push_back("SerialNumber");
        vs.push_back("ID");
        vs.push_back("PosX");
        vs.push_back("PosY");
        vs.push_back("PosZ");
        vs.push_back("RotX");
        vs.push_back("RotY");
        vs.push_back("RotZ");
        vs.push_back(u8"残りHP");
        vs.push_back(u8"討伐済み");
        vvs.push_back(vs);
        vs.clear();

        for (size_t i = 0; i < enemyList.size(); ++i)
        {
            vs.clear();
            auto& enemyInfo = enemyList.at(i);

            vs.push_back(std::to_string(i + 1));
            vs.push_back(enemyInfo.m_id);
            vs.push_back(std::to_string(enemyInfo.m_pos.m_x));
            vs.push_back(std::to_string(enemyInfo.m_pos.m_y));
            vs.push_back(std::to_string(enemyInfo.m_pos.m_z));
            vs.push_back("0");
            vs.push_back("0");
            vs.push_back("0");
            vs.push_back(std::to_string(enemyInfo.m_HP));
            vs.push_back("");
            vvs.push_back(vs);
        }

        csv::Write("enemy.csv", vvs);
    }
}

