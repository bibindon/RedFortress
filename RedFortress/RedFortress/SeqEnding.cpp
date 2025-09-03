#include "SeqEnding.h"
#include "KeyBoard.h"
#include "Mouse.h"
#include "GamePad.h"
#include "Common.h"
#include "SharedObj.h"
#include "BGM.h"
#include "SoundEffect.h"
#include "QuestManager.h"

#include <cassert>

using namespace NSStoryTelling;

namespace NSStoryTelling
{
class Sprite : public ISprite
{
public:

    Sprite(LPDIRECT3DDEVICE9 dev)
        : m_pD3DDevice(dev)
    {
    }

    void DrawImage(const int x, const int y, const int transparency) override
    {
        D3DXVECTOR3 pos { (float)x, (float)y, 0.f };
        m_D3DSprite->Begin(D3DXSPRITE_ALPHABLEND);
        RECT rect = {
            0,
            0,
            static_cast<LONG>(m_width),
            static_cast<LONG>(m_height) };
        D3DXVECTOR3 center { 0, 0, 0 };
        m_D3DSprite->Draw(
            m_pD3DTexture,
            &rect,
            &center,
            &pos,
            D3DCOLOR_ARGB(transparency, 255, 255, 255));
        m_D3DSprite->End();

    }

    void Load(const std::wstring& filepath) override
    {
        LPD3DXSPRITE tempSprite { nullptr };
        if (FAILED(D3DXCreateSprite(m_pD3DDevice, &m_D3DSprite)))
        {
            throw std::exception("Failed to create a sprite.");
        }

        if (FAILED(D3DXCreateTextureFromFile(
            m_pD3DDevice,
            filepath.c_str(),
            &m_pD3DTexture)))
        {
            throw std::exception("Failed to create a texture.");
        }

        D3DSURFACE_DESC desc { };
        if (FAILED(m_pD3DTexture->GetLevelDesc(0, &desc)))
        {
            throw std::exception("Failed to create a texture.");
        }
        m_width = desc.Width;
        m_height = desc.Height;
    }

    ISprite* Create()
    {
        return new Sprite(m_pD3DDevice);
    }

    ~Sprite()
    {
        if (m_D3DSprite != nullptr)
        {
            m_D3DSprite->Release();
        }

        if (m_pD3DTexture != nullptr)
        {
            m_pD3DTexture->Release();
        }
    }

private:

    LPDIRECT3DDEVICE9 m_pD3DDevice = NULL;
    LPD3DXSPRITE m_D3DSprite = NULL;
    LPDIRECT3DTEXTURE9 m_pD3DTexture = NULL;
    UINT m_width { 0 };
    UINT m_height { 0 };
};

class Font : public IFont
{
public:

    Font(LPDIRECT3DDEVICE9 pD3DDevice)
        : m_pD3DDevice(pD3DDevice)
    {
    }

    void Init(const bool bEnglish)
    {
        HRESULT hr = S_OK;
        if (!bEnglish)
        {
            hr = D3DXCreateFont(m_pD3DDevice,
                                24,
                                0,
                                FW_NORMAL,
                                1,
                                false,
                                SHIFTJIS_CHARSET,
                                OUT_TT_ONLY_PRECIS,
                                CLEARTYPE_NATURAL_QUALITY,
                                FF_DONTCARE,
                                _T("BIZ UDMincho"),
                                &m_pFont);
        }
        else
        {
            hr = D3DXCreateFont(m_pD3DDevice,
                                Common::FontSizeEng(),
                                0,
                                Common::FontThicknessEng(),
                                1,
                                false,
                                DEFAULT_CHARSET,
                                OUT_TT_ONLY_PRECIS,
                                CLEARTYPE_NATURAL_QUALITY,
                                FF_DONTCARE,
                                Common::FontNameEng(),
                                &m_pFont);
        }

        assert(hr == S_OK);
    }

    virtual void DrawText_(const std::wstring& msg, const int x, const int y)
    {
        RECT rect = { x, y, 0, 0 };
        m_pFont->DrawText(NULL, msg.c_str(), -1, &rect, DT_LEFT | DT_NOCLIP,
            D3DCOLOR_ARGB(255, 255, 255, 255));
    }

    ~Font()
    {
        m_pFont->Release();
        m_pFont = nullptr;
    }

private:

    LPDIRECT3DDEVICE9 m_pD3DDevice = NULL;
    LPD3DXFONT m_pFont = NULL;
};

class SoundEffect : public ISoundEffect
{
    virtual void PlayMove() override
    {
        ::SoundEffect::get_ton()->play(_T("res\\sound\\cursor_move.wav"));
    }

    virtual void Init() override
    {
        ::SoundEffect::get_ton()->load(_T("res\\sound\\cursor_move.wav"));
    }
};
}

SeqEnding::SeqEnding()
{

    m_spriteEnd = NEW ::Sprite(_T("res\\image\\ending11.png"));

    BGMManager::Get()->SetEnding(true);
    {
        ISoundEffect* pSE = NEW NSStoryTelling::SoundEffect();

        ISprite* sprTextBack = NEW NSStoryTelling::Sprite(SharedObj::GetD3DDevice());
        sprTextBack->Load(_T("res\\image\\textBack.png"));

        ISprite* sprFade = NEW NSStoryTelling::Sprite(SharedObj::GetD3DDevice());
        sprFade->Load(_T("res\\image\\black.png"));

        NSStoryTelling::IFont* pFont = NEW NSStoryTelling::Font(SharedObj::GetD3DDevice());

        auto finished = QuestManager::Get()->GetQuestFinished(L"Q8500");

        if (finished)
        {
            m_bTrueEnding = true;
        }

        std::vector<Page> pageList;

        if (m_bTrueEnding)
        {
            if (!SharedObj::IsEnglish())
            {
                Page page;
                ISprite* sprite = NEW NSStoryTelling::Sprite(SharedObj::GetD3DDevice());
                sprite->Load(_T("res\\image\\ending01.png"));
                page.SetSprite(sprite);
                std::vector<std::vector<std::wstring> > vvs;
                std::vector<std::wstring> vs;
                vs.push_back(_T("数時間後、ホシマンは目を覚まし砂浜から内陸の方に歩いていった。"));
                vs.push_back(_T("しばらくして道路を見つけた。"));
                vs.push_back(_T("道路を歩き続けるとガソリンスタンドがあった。"));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("そこでホシマンは今いる場所がジーフォーク共和国であることを確認した。"));
                vs.push_back(_T("北西に進んでいたつもりだったが南に相当ずれていた。"));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("その日からホシマンは法律を無視して救助を最優先することにした。"));
                vvs.push_back(vs);

                // 家のカギがない場合は一言添える
                {
                    auto homeKey = Common::Inventory()->GetSubIdList(L"homeKey");
                    if (homeKey.empty())
                    {
                        vs.clear();
                        vs.push_back(_T("ホシマンはハッとして持ち物を漁った。"));
                        vs.push_back(_T("（家のカギをなくした・・・。くそ・・・。）"));
                        vvs.push_back(vs);
                    }
                }

                page.SetTextList(vvs);
                pageList.push_back(page);
            }
            else
            {
                Page page;
                ISprite* sprite = NEW NSStoryTelling::Sprite(SharedObj::GetD3DDevice());
                sprite->Load(_T("res\\image\\ending01.png"));
                page.SetSprite(sprite);
                std::vector<std::vector<std::wstring> > vvs;
                std::vector<std::wstring> vs;
                vs.push_back(_T("After a few hours, Starman woke up and walked inland from the beach."));
                vs.push_back(_T("Before long, he came across a paved road."));
                vs.push_back(_T("He followed the road and eventually reached a gas station."));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("There he confirmed he was in the Republic of GEFORQE."));
                vs.push_back(_T("He thought he’d been heading northwest, but had drifted far south."));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("From that day on, he decided to ignore the law and put rescue above all else."));
                vvs.push_back(vs);

                // If he doesn’t have the house key, add one more line
                {
                    auto homeKey = Common::Inventory()->GetSubIdList(L"homeKey");
                    if (homeKey.empty())
                    {
                        vs.clear();
                        vs.push_back(_T("Startled, Starman rummaged through his belongings."));
                        vs.push_back(_T("(I lost my house key... Damn it...)"));
                        vvs.push_back(vs);
                    }
                }

                page.SetTextList(vvs);
                pageList.push_back(page);
            }

            if (!SharedObj::IsEnglish())
            {
                Page page;
                ISprite* sprite = NEW NSStoryTelling::Sprite(SharedObj::GetD3DDevice());
                sprite->Load(_T("res\\image\\ending02.png"));
                page.SetSprite(sprite);
                std::vector<std::vector<std::wstring> > vvs;
                std::vector<std::wstring> vs;
                vs.push_back(_T("３か月後、８月３１日。"));
                vs.push_back(_T("ホシマンは深夜の海岸にいた。"));
                vs.push_back(_T("ホシマンはラデオウ連邦に密入国していた。政府に頼るわけにはいかなかった。"));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("プロリタン島は国家的な犯罪の舞台となっているため、政府が救助に協力するか分からなかった。"));
                vs.push_back(_T("それどころかホシマンが暗殺されてしまう可能性もあるとホシマンは考えた。"));
                vs.push_back(_T("島には危険人物が二人いるため急いで救出に向かう必要もあった。自分で向かうしかなかった。"));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("モーターボートに乗るための免許は二日でとることができ、誰でも受かるようだった。"));
                vs.push_back(_T("しかし陸から９ｋｍしか運転できなかった。"));
                vs.push_back(_T("ラデオウ連邦からプロリタン島までは１４０ｋｍあり、個人が合法的に行くことはほぼ不可能だった。"));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("小型船舶の免許を取得するにもホシマンは密入国者だった。"));
                vs.push_back(_T("ホシマンは借金をして中古のモーターボートをネットで購入した。"));
                vs.push_back(_T("そして自家用車にボート取付用の器具を取り付け、モーターボートを取り付けた。"));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("さらに海上でも現在地を確認できるＧＰＳ受信機を購入した。"));
                vs.push_back(_T("日に日に無人島での生活はホシマンにとって過去の記憶となっていき、"));
                vs.push_back(_T("記憶の中のダイケイマンとサンカクマンの声、形が曖昧となっていた。"));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("今も二人はプロリタン島にいる。命がけの逃亡生活をしているかもしれない。"));
                vs.push_back(_T("どちらかは悪魔と契約しているかもしれない。"));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("決意が薄れ、このまま二人の救出をやめて普通の生活をし始めてしまうのではないか、"));
                vs.push_back(_T("そのことがホシマンは恐怖だった。"));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("そして、８月３１日まで息を潜め、８月３１日に海岸までやってきた。"));
                vs.push_back(_T("最悪、二人を救出した後なら、警察に捕まっても良い。警察に捕まれば助かるからだ。"));
                vs.push_back(_T("ホシマンは郵便ポストに封筒を数枚いれた。"));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("封筒のあて先は存在しない住所であり、送り主が警察やマスコミとなっていた。"));
                vs.push_back(_T("存在しない住所が書かれた郵便物は送り主のもとへ帰ってくる。"));
                vs.push_back(_T("そして、送り主のもとへ帰ってくるのは１～２週間かかる。"));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("ホシマン単独による救助が失敗し、死亡した場合、プロリタン島に生存者がいるということを誰も知らなくなってしまう。"));
                vs.push_back(_T("かと言って、警察に頼れば救助自体ができなくなる可能性があった。"));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("ホシマンが救助に失敗した場合、警察に郵便物が届き、"));
                vs.push_back(_T("ホシマンが死亡した場合でもプロリタン島に生存者いるという情報が残せる、という作戦だった。"));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("郵便物には、生存者がいるということだけが書いてあり、差出人や生存者の情報は載せていなかった。"));
                vs.push_back(_T("もし救出に成功すればそのままで問題なかった。"));
                vs.push_back(_T("ホシマンは意を決してプロリタン島に向かって出発した。"));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("何もない真っ暗な海をモーターボートで進んだ。モーターボートの最高速度は時速２０ｋｍ程度。"));
                vs.push_back(_T("時々、ＧＰＳ受信機で現在地を確認した。"));
                vs.push_back(_T("イカダで航海したときとは打って変わって危なげなく目的地まで到達できそうだった。"));
                page.SetTextList(vvs);
                pageList.push_back(page);
            }
            else
            {
                Page page;
                ISprite* sprite = NEW NSStoryTelling::Sprite(SharedObj::GetD3DDevice());
                sprite->Load(_T("res\\image\\ending02.png"));
                page.SetSprite(sprite);
                std::vector<std::vector<std::wstring> > vvs;
                std::vector<std::wstring> vs;
                vs.push_back(_T("Three months later, August 31."));
                vs.push_back(_T("Starman stood on the beach in the middle of the night."));
                vs.push_back(_T("He had illegally entered the Radeou Federation; relying on the authorities was impossible."));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("Because Prolitan Island was the stage of a national-level crime, he doubted any government would help with a rescue."));
                vs.push_back(_T("Starman even feared he might be assassinated."));
                vs.push_back(_T("Two dangerous people were on the island, so he had to hurry there and rescue them himself."));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("A motorboat license could be earned in two days, and almost anyone passed."));
                vs.push_back(_T("However, it allowed travel only within nine kilometers of shore."));
                vs.push_back(_T("Prolitan Island lay 140 kilometers from the Radeou Federation, making a legal trip nearly impossible."));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("Since he was an illegal entrant, applying for a small-vessel license was not an option."));
                vs.push_back(_T("Starman went into debt and bought a used motorboat online."));
                vs.push_back(_T("He then fitted his car with a trailer hitch and mounted the boat."));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("He also bought a GPS receiver so he could track his position at sea."));
                vs.push_back(_T("Day by day, life on the deserted island faded into distant memory,"));
                vs.push_back(_T("and the voices and faces of Trapezoid Man and Triangle Man grew hazy."));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("The two were likely still on Prolitan Island, perhaps living on the run."));
                vs.push_back(_T("One of them might even have struck a deal with a demon."));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("He feared his resolve would weaken and he would simply return to an ordinary life without rescuing them,"));
                vs.push_back(_T("and that possibility terrified Starman."));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("So he laid low until August 31, then made his way to the coast."));
                vs.push_back(_T("At worst, once the rescue was done, being arrested would be fine—getting caught meant survival."));
                vs.push_back(_T("Starman slipped several envelopes into a mailbox."));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("Each envelope was addressed to a nonexistent location and listed the police or the press as the sender."));
                vs.push_back(_T("Mail sent to a fake address is returned to the sender,"));
                vs.push_back(_T("and that return takes about one to two weeks."));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("If his solo rescue failed and he died, no one would learn there were survivors on Prolitan Island."));
                vs.push_back(_T("Yet asking the police outright might make the rescue impossible."));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("His plan was that, if he failed, the mail would reach the police,"));
                vs.push_back(_T("so the fact that survivors existed on Prolitan Island would remain even if he died."));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("The letters said only that survivors were there; no sender or survivor details were given."));
                vs.push_back(_T("If the rescue succeeded, they could simply be ignored."));
                vs.push_back(_T("Steeling himself, Starman set out for Prolitan Island."));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("He guided the motorboat across a pitch-black, empty sea at roughly twenty kilometers per hour."));
                vs.push_back(_T("From time to time he checked the GPS to confirm his position."));
                vs.push_back(_T("Unlike his raft voyage, it seemed he would reach the island without incident."));
                page.SetTextList(vvs);
                pageList.push_back(page);
            }

            if (!SharedObj::IsEnglish())
            {
                Page page;
                ISprite* sprite = NEW NSStoryTelling::Sprite(SharedObj::GetD3DDevice());
                sprite->Load(_T("res\\image\\ending03.png"));
                page.SetSprite(sprite);
                std::vector<std::vector<std::wstring> > vvs;
                std::vector<std::wstring> vs;
                vs.clear();
                vs.push_back(_T("翌朝、ホシマンはダイレクトネクス海岸洞窟の辺りに到着した。"));
                vs.push_back(_T("ボートで海岸洞窟に近づくと人の姿が見えた。"));
                vs.push_back(_T("サンカクマンだった。サンカクマンはこちらに気づき手を振った。"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("サンカクマンはそのあと、一度、奥に引っ込み、そのあと、ダイケイマンが杖を突きながら出てきた。"));
                vs.push_back(_T("ホシマンは海岸洞窟に到着すると、ダイケイマンたちに手を振ってから、水上ボートをロープで岩にくくりつけた。"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("「それが流れて行っちゃったら大変だよ」"));
                vs.push_back(_T("ダイケイマンは笑いながら言った。"));
                vs.push_back(_T("「ははは」"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("「冗談よしてくれ」"));
                vs.push_back(_T("ホシマンは海から上がり砂浜に到着した。ダイケイマンとサンカクマンも砂浜にやってきた。"));
                vs.push_back(_T("「よかった・・・ほんとうによかったよ・・・」"));
                vvs.push_back(vs);
                page.SetTextList(vvs);
                pageList.push_back(page);
            }
            else
            {
                Page page;
                ISprite* sprite = NEW NSStoryTelling::Sprite(SharedObj::GetD3DDevice());
                sprite->Load(_T("res\\image\\ending03.png"));
                page.SetSprite(sprite);
                std::vector<std::vector<std::wstring> > vvs;
                std::vector<std::wstring> vs;

                vs.clear();
                vs.push_back(_T("The next morning, Starman reached the area near Directnex Coastal Cave."));
                vs.push_back(_T("As he brought the boat closer to the cave, he spotted someone."));
                vs.push_back(_T("It was Triangle Man, who noticed him and waved."));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("Triangle Man slipped back inside for a moment, then Trapezoid Man emerged, leaning on his staff."));
                vs.push_back(_T("When Starman pulled up to the cave, he waved to them before tying the motorboat to a rock with a rope."));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("\"It'd be a disaster if that drifted away,\""));
                vs.push_back(_T("Trapezoid Man said with a laugh."));
                vs.push_back(_T("\"Ha ha ha.\""));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("\"Give me a break,\""));
                vs.push_back(_T("Starman climbed out of the water and stepped onto the sand. Trapezoid Man and Triangle Man joined him there."));
                vs.push_back(_T("\"I'm so glad... truly glad...\""));
                vvs.push_back(vs);

                page.SetTextList(vvs);
                pageList.push_back(page);
            }

            if (!SharedObj::IsEnglish())
            {
                Page page;
                ISprite* sprite = NEW NSStoryTelling::Sprite(SharedObj::GetD3DDevice());
                sprite->Load(_T("res\\image\\ending04.png"));
                page.SetSprite(sprite);
                std::vector<std::vector<std::wstring> > vvs;
                std::vector<std::wstring> vs;
                vs.clear();
                vs.push_back(_T("「待ってたぜー！」"));
                vs.push_back(_T("「歩けるようになったんだな」"));
                vs.push_back(_T("「おー、杖があればね」"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("「・・・。"));
                vs.push_back(_T("　あいつらは来なかったか？」"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("「来たよ。両方とも餓死寸前の状態でな。ガインの方は夜中に泥棒に来た。"));
                vs.push_back(_T("  普通に『食べ物をくれ』って言ってくれればいいのに。"));
                vs.push_back(_T("  こっちの姿に気づいたら逃げて行った。そのあとは会ってない。」"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("「ネスの方は、まぁ・・・いつもの感じで脅迫してきたから追い返した。二人がどうなってるかはわからない」"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("「わかった。すまないが警察が来るかもしれないから急いで帰ろう。一般人がここに来るのは違法なんだ」"));
                vs.push_back(_T("「お、　おーけー」"));
                vs.push_back(_T("「シカクマンの墓、見ていくか？」"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("「・・・。"));
                vs.push_back(_T("　あ、あぁ・・・」"));
                vs.push_back(_T("海岸洞窟を通り過ぎ、内陸の方に行くと、以前と変わらない姿でシカクマンの墓があった。"));
                vvs.push_back(vs);
                page.SetTextList(vvs);
                pageList.push_back(page);
            }
            else
            {
                Page page;
                ISprite* sprite = NEW NSStoryTelling::Sprite(SharedObj::GetD3DDevice());
                sprite->Load(_T("res\\image\\ending04.png"));
                page.SetSprite(sprite);
                std::vector<std::vector<std::wstring> > vvs;
                std::vector<std::wstring> vs;

                // ――― Greeting ―――
                vs.clear();
                vs.push_back(_T("\"We've been waiting for you!\""));
                vs.push_back(_T("\"Looks like you can walk again.\""));
                vs.push_back(_T("\"Yeah—so long as I've got this cane.\""));
                vvs.push_back(vs);

                // ――― Asking about the others ―――
                vs.clear();
                vs.push_back(_T("\"...\""));
                vs.push_back(_T("\"So they never came, huh?\""));
                vvs.push_back(vs);

                // ――― Report on Gain ―――
                vs.clear();
                vs.push_back(_T("\"They did show up. Both of them were on the brink of starvation."));
                vs.push_back(_T("  Gain tried to steal food in the middle of the night."));
                vs.push_back(_T("  He could have just asked, but when he saw us he bolted—haven't seen him since.\""));
                vvs.push_back(vs);

                // ――― Report on Nes ―――
                vs.clear();
                vs.push_back(_T("\"As for Nes, well... the usual threats, so I sent him packing."));
                vs.push_back(_T("No idea what’s become of either of them.\""));
                vvs.push_back(vs);

                // ――― Hurry back & grave suggestion ―――
                vs.clear();
                vs.push_back(_T("\"Understood. Sorry, but the police might show up, so we need to head back fast. Civilians aren’t allowed out here.\""));
                vs.push_back(_T("\"O-okay.\""));
                vs.push_back(_T("\"Want to stop by Square Man’s grave first?\""));
                vvs.push_back(vs);

                // ――― Visiting Square Man’s grave ―――
                vs.clear();
                vs.push_back(_T("\"...\""));
                vs.push_back(_T("\"Y-yeah...\""));
                vs.push_back(_T("Passing the coastal cave and moving inland, they found Square Man’s grave exactly as they remembered it."));
                vvs.push_back(vs);

                page.SetTextList(vvs);
                pageList.push_back(page);
            }

            if (!SharedObj::IsEnglish())
            {
                Page page;
                ISprite* sprite = NEW NSStoryTelling::Sprite(SharedObj::GetD3DDevice());
                sprite->Load(_T("res\\image\\ending05.png"));
                page.SetSprite(sprite);
                std::vector<std::vector<std::wstring> > vvs;
                std::vector<std::wstring> vs;
                vs.push_back(_T("ホシマンは墓の前で正座し、手を合わせて目をつぶった。"));
                vs.push_back(_T("「「「・・・」」」"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("「じゃあ・・・、行こう」"));
                vs.push_back(_T("ホシマンは立ち上がり、３人でモーターボートのところに向かった。"));
                vvs.push_back(vs);
                page.SetTextList(vvs);
                pageList.push_back(page);
            }
            else
            {
                Page page;
                ISprite* sprite = NEW NSStoryTelling::Sprite(SharedObj::GetD3DDevice());
                sprite->Load(_T("res\\image\\ending05.png"));
                page.SetSprite(sprite);
                std::vector<std::vector<std::wstring> > vvs;
                std::vector<std::wstring> vs;

                vs.push_back(_T("Starman knelt before the grave, pressed his hands together, and closed his eyes."));
                vs.push_back(_T("\"\"\"...\"\"\""));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("\"Alright... let's go.\""));
                vs.push_back(_T("Starman rose to his feet, and the three of them headed back toward the motorboat."));
                vvs.push_back(vs);

                page.SetTextList(vvs);
                pageList.push_back(page);
            }

            if (!SharedObj::IsEnglish())
            {
                Page page;
                ISprite* sprite = NEW NSStoryTelling::Sprite(SharedObj::GetD3DDevice());
                sprite->Load(_T("res\\image\\ending06.png"));
                page.SetSprite(sprite);
                std::vector<std::vector<std::wstring> > vvs;
                std::vector<std::wstring> vs;
                vs.clear();
                vs.push_back(_T("「今も、あの、謎の草は食べてる？」"));
                vs.push_back(_T("ホシマンはダイケイマンたちに聞いた。"));
                vs.push_back(_T("「おう」"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("「そうだ、あの草ってなんだったの？」"));
                vs.push_back(_T("ダイケイマンたちはホシマンに聞いた。"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("「多分、アカツメクサと・・・カキドオシと・・・」"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("「カラスノエンドウ、オオバコ、カワラヨモギ、クズ、だ。"));
                vs.push_back(_T("　しっかり覚えてきた」"));
                vs.push_back(_T("ホシマンがかすかに笑った。"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("「う、うん」サンカクマンは返事した。"));
                vs.push_back(_T("「ごめん、聞いといてなんだけど、全然わからん」"));
                vs.push_back(_T("ダイケイマンは笑って答えた。"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("モーターボートの中にはタンクが８個あった。"));
                vs.push_back(_T("「船は、実は燃費がすごく悪いんだ。１リットルで２Ｋｍくらいしか進めない。"));
                vs.push_back(_T("  １６０リットル持ってきた。ガソリン入れるから待っててくれ」"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("モーターボート内のガソリンの残量は１割以下、すでにガソリン携行缶は一つが空になっていた。"));
                vs.push_back(_T("ホシマンは２０リットルガソリン携行缶を３つ使いガソリンを満タンにした。"));
                vs.push_back(_T("「帰りは、乗員が３人で海流が進行方向と逆向きになる。それを加味すると８０リットルくらいで足りるはずだ」"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("「わかった」"));
                vs.push_back(_T("「乗ってくれ。１０時間くらいかかる。食料と水はボートの中に用意してある」"));
                vs.push_back(_T("３人はモーターボートに乗った。"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("「久しぶりのお菓子だ」"));
                vs.push_back(_T("「小さくて軽くてカロリーの高いものにしたから、お菓子ばかりだ。好きなだけ食べてくれ」"));
                vs.push_back(_T("ガソリンの匂いが充満する。鍵を回すとエンジンの力強い音がした。"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("（たのむ・・・このまま無事帰らせてくれ・・・）"));
                vs.push_back(_T("縄を外し、ホシマンたちは出発した。"));
                vs.push_back(_T("ホシマンはGPS装置を確認しながら進んだ。"));
                vvs.push_back(vs);
                page.SetTextList(vvs);
                pageList.push_back(page);
            }
            else
            {
                Page page;
                ISprite* sprite = NEW NSStoryTelling::Sprite(SharedObj::GetD3DDevice());
                sprite->Load(_T("res\\image\\ending06.png"));
                page.SetSprite(sprite);
                std::vector<std::vector<std::wstring> > vvs;
                std::vector<std::wstring> vs;

                // ―― Fuel preparation ――
                vs.clear();
                vs.push_back(_T("There were eight fuel tanks on the motorboat."));
                vs.push_back(_T("\"Boats burn fuel like crazy—only about 2 km per liter."));
                vs.push_back(_T("I brought 160 liters. Let me top her off, so hang tight.\""));
                vvs.push_back(vs);

                // ―― Refueling & calculation ――
                vs.clear();
                vs.push_back(_T("Less than ten percent of the boat’s fuel remained, and one jerry can was already empty."));
                vs.push_back(_T("Starman used three 20-liter cans to fill the tank to the brim."));
                vs.push_back(_T("\"The trip back will have three passengers and the current against us, so about 80 liters should be enough.\""));
                vvs.push_back(vs);

                // ―― Boarding ――
                vs.clear();
                vs.push_back(_T("\"Got it.\""));
                vs.push_back(_T("\"Climb aboard. It’ll take roughly ten hours. Food and water are stowed inside.\""));
                vs.push_back(_T("The three of them boarded the motorboat."));
                vvs.push_back(vs);

                // ―― Snacks & departure ――
                vs.clear();
                vs.push_back(_T("\"Snacks—haven’t had those in ages.\""));
                vs.push_back(_T("\"Everything’s small, light, and calorie-dense, so it’s mostly sweets. Eat as much as you like.\""));
                vs.push_back(_T("The smell of gasoline filled the air. When he turned the key, the engine roared to life."));
                vvs.push_back(vs);

                // ―― Silent prayer & launch ――
                vs.clear();
                vs.push_back(_T("(Please… let us get home safely…)"));
                vs.push_back(_T("They cast off the rope and set out."));
                vs.push_back(_T("Starman checked the GPS as they pushed forward."));
                vvs.push_back(vs);

                page.SetTextList(vvs);
                pageList.push_back(page);
            }

            if (!SharedObj::IsEnglish())
            {
                Page page;
                ISprite* sprite = NEW NSStoryTelling::Sprite(SharedObj::GetD3DDevice());
                sprite->Load(_T("res\\image\\ending07.png"));
                page.SetSprite(sprite);
                std::vector<std::vector<std::wstring> > vvs;
                std::vector<std::wstring> vs;

                vs.push_back(_T("「うぉ～すげぇ～」"));
                vs.push_back(_T("「１年前に船に乗った時を思い出すなぁ～」"));
                vs.push_back(_T("「・・・」"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("ダイケイマンとサンカクマンは潮風を堪能していた。後ろを見るとすでに遠くなっている島がみえた。"));
                vs.push_back(_T("サンカクマンは海岸に角の生えた人が立っていることに気づいた。サンカクマンは手を振った。"));
                vvs.push_back(vs);
                page.SetTextList(vvs);
                pageList.push_back(page);
            }
            else
            {
                Page page;
                ISprite* sprite = NEW NSStoryTelling::Sprite(SharedObj::GetD3DDevice());
                sprite->Load(_T("res\\image\\ending07.png"));
                page.SetSprite(sprite);
                std::vector<std::vector<std::wstring> > vvs;
                std::vector<std::wstring> vs;

                // ボート上での第一声
                vs.push_back(_T("\"Whoa—amazing!\""));
                vs.push_back(_T("\"This takes me back to when we boarded a boat a year ago.\""));
                vs.push_back(_T("\"...\""));
                vvs.push_back(vs);

                // 後方の島と岸の人物に気づく場面
                vs.clear();
                vs.push_back(_T("Trapezoid Man and Triangle Man soaked up the salty breeze. When they glanced back, the island was already far in the distance."));
                vs.push_back(_T("Triangle Man spotted someone with horns standing on the shore and waved to them."));
                vvs.push_back(vs);

                page.SetTextList(vvs);
                pageList.push_back(page);
            }

            if (!SharedObj::IsEnglish())
            {
                Page page;
                ISprite* sprite = NEW NSStoryTelling::Sprite(SharedObj::GetD3DDevice());
                sprite->Load(_T("res\\image\\ending08.png"));
                page.SetSprite(sprite);
                std::vector<std::vector<std::wstring> > vvs;
                std::vector<std::wstring> vs;
                vs.push_back(_T("９時間後、３人はラデオウ連邦の本土に到着した。"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("３人はホシマンの家で一泊することにした。"));
                vs.push_back(_T("ホシマンの家に到着したあとサンカクマンは母方の祖父母の家に電話した。"));
                vs.push_back(_T("迎えに来てもらうことになった。"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("そこでサンカクマンの両親が１年前の海難事故によって亡くなっていることを知った。"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("翌日、ホシマンの家に老夫婦が車でやってきた。サンカクマンは老夫婦と共に帰っていった。"));
                vs.push_back(_T("それから、ホシマンはダイケイマンを連れて病院に行った。"));
                vs.push_back(_T("そこで去年の海難事故の生存者が帰還している、ということが明るみになった。"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("ホシマンたちは警察の事情聴取を受けることになった。"));
                vvs.push_back(vs);
                page.SetTextList(vvs);
                pageList.push_back(page);
            }
            else
            {
                Page page;
                ISprite* sprite = NEW NSStoryTelling::Sprite(SharedObj::GetD3DDevice());
                sprite->Load(_T("res\\image\\ending08.png"));
                page.SetSprite(sprite);
                std::vector<std::vector<std::wstring> > vvs;
                std::vector<std::wstring> vs;

                // ―― 本土到着 ――
                vs.push_back(_T("Nine hours later, the three arrived on the mainland of the Radeou Federation."));
                vvs.push_back(vs);

                // ―― ホシマン宅で一泊・電話 ――
                vs.clear();
                vs.push_back(_T("They decided to spend the night at Starman's house."));
                vs.push_back(_T("After arriving, Triangle Man called his maternal grandparents."));
                vs.push_back(_T("They agreed to pick him up."));
                vvs.push_back(vs);

                // ―― 両親が事故で死亡と判明 ――
                vs.clear();
                vs.push_back(_T("There he learned that his parents had died in the maritime accident a year earlier."));
                vvs.push_back(vs);

                // ―― 翌日・老夫婦来訪・病院へ ――
                vs.clear();
                vs.push_back(_T("The next day an elderly couple arrived by car; Triangle Man left with them."));
                vs.push_back(_T("After that, Starman took Trapezoid Man to the hospital."));
                vs.push_back(_T("News spread that survivors of last year's shipwreck had finally returned."));
                vvs.push_back(vs);

                // ―― 警察の事情聴取 ――
                vs.clear();
                vs.push_back(_T("Starman and the others were summoned for questioning by the police."));
                vvs.push_back(vs);

                page.SetTextList(vvs);
                pageList.push_back(page);
            }

            if (!SharedObj::IsEnglish())
            {
                Page page;
                ISprite* sprite = NEW NSStoryTelling::Sprite(SharedObj::GetD3DDevice());
                sprite->Load(_T("res\\image\\ending09.png"));
                page.SetSprite(sprite);
                std::vector<std::vector<std::wstring> > vvs;
                std::vector<std::wstring> vs;
                vs.push_back(_T("１年後、サンカクマンの家の近くの喫茶店に３人が集まった。"));
                vs.push_back(_T("あれからSNSで連絡を取り合っていたが久しぶりに会うことにしたのであった。"));
                vs.push_back(_T("１年前、ラデオウ連邦に帰還したとき、ホシマンは警察に逮捕された。そして事情聴取を受けた後、釈放となった。"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("オプティプル島へ向かう船が沈没した本当の原因は、"));
                vs.push_back(_T("船の修繕費として政府から送られていた助成金を公務員が懐に入れていたことだった。"));
                vs.push_back(_T("船は修理されずに修理されたことになっていた。"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("ラデオウ連邦ではジャーナリストが政府によって暗殺されることが日常だったため"));
                vs.push_back(_T("ホシマンたちの帰還は小さく報じられた。"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("また、高度に文明が発展した情報化社会において、"));
                vs.push_back(_T("『無人島からの生還』というニュースは毎秒追加され続けるネットニュースによって、すぐに埋もれた。"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("ホシマンは借金を返済するため、倉庫で働くようになっていた。"));
                vs.push_back(_T("ダイケイマンは複数回に及ぶ骨の整形手術を受け、ついに歩けるようになっていた。"));
                vs.push_back(_T("サンカクマンは母方の祖父母の家に住むことになり小学６年生となっていた。"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("喫茶店に集まった３人はメニューを見ていた。"));
                vs.push_back(_T("「これを注文するしかね～な～」"));
                vs.push_back(_T("ダイケイマンはメニューの写真を見せた。"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("サンカクマンとホシマンが顔を近づけると、写真のメニューはパエリアだった。"));
                vs.push_back(_T("「結局、あの黒い貝は本当にムール貝だったらしい」"));
                vs.push_back(_T("ホシマンが言った。"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("「味比べしてみよう。パエリアのさ、ご飯の中に貝殻とかエビの殻が入ってるの、ちょっと嫌だったんだよね」"));
                vs.push_back(_T("「今は？」"));
                vs.push_back(_T("「うーん・・・」"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("「無人島生活で泥まみれのものをあんなに食べてたのに・・・」"));
                vs.push_back(_T("「そうなんだけどなぁ～」"));
                vs.push_back(_T("やがてパエリア、ナタデココ入りのデザート、フルーツが運ばれてきた。"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("「『なんでこいつら果物ばっかり注文するんだろう』って思われてそう」"));
                vs.push_back(_T("ダイケイマンが言った。"));
                vs.push_back(_T("３人はムール貝を食べた。"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("「同じだ～・・・、まーでも、無人島で食べたやつには叶わないね～」"));
                vs.push_back(_T("サンカクマンは得意げに言った。"));
                vs.push_back(_T("ホシマンはあの日のことを思い出していた。"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("「徹夜で森を歩いて、村役場から海岸洞窟に行ったことがあった。"));
                vs.push_back(_T("　あの日は、夜に海岸洞窟について、それで焚き火を４人で囲んで、"));
                vs.push_back(_T("　この貝を食べながらカードゲームをした。懐かしいな」"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("「・・・。"));
                vs.push_back(_T("　次集まるときは３人で砂浜に行くか？」"));
                vs.push_back(_T("「行きたい！」"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("「・・・」"));
                vs.push_back(_T("３人はお互いの顔を見た。"));
                vs.push_back(_T("「決まりだな」"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("ホシマンは言った。"));
                vvs.push_back(vs);
                page.SetTextList(vvs);
                pageList.push_back(page);
            }
            else
            {
                Page page;
                ISprite* sprite = NEW NSStoryTelling::Sprite(SharedObj::GetD3DDevice());
                sprite->Load(_T("res\\image\\ending09.png"));
                page.SetSprite(sprite);
                std::vector<std::vector<std::wstring> > vvs;
                std::vector<std::wstring> vs;

                // ―― 再会シーン ――
                vs.push_back(_T("One year later, the three met at a café near Triangle Man's house."));
                vs.push_back(_T("They had stayed in touch through social media, but decided it was time to see each other again."));
                vs.push_back(_T("When they returned to the Radeou Federation a year ago, Starman was arrested, questioned, and later released."));
                vvs.push_back(vs);

                // ―― 沈没事故の真相 ――
                vs.clear();
                vs.push_back(_T("The real reason the ship heading to Optipl Island sank"));
                vs.push_back(_T("was that officials pocketed the subsidy the government had allocated for repairs."));
                vs.push_back(_T("The vessel was listed as fixed, but no work had ever been done."));
                vvs.push_back(vs);

                // ―― 帰還報道が小さい理由 ――
                vs.clear();
                vs.push_back(_T("Because government-ordered journalist assassinations were routine in the Radeou Federation,"));
                vs.push_back(_T("news of their return received only minimal coverage."));
                vvs.push_back(vs);

                // ―― ニュースが埋もれる情報社会 ――
                vs.clear();
                vs.push_back(_T("In a hyper-connected information society,"));
                vs.push_back(_T("the headline \"Survivors Return from Deserted Island\" was quickly buried by the endless flow of online news."));
                vvs.push_back(vs);

                // ―― ３人の近況 ――
                vs.clear();
                vs.push_back(_T("To repay his debts, Starman had taken a warehouse job."));
                vs.push_back(_T("Trapezoid Man underwent several orthopedic surgeries and could finally walk."));
                vs.push_back(_T("Triangle Man now lived with his maternal grandparents and was in sixth grade."));
                vvs.push_back(vs);

                // ―― 喫茶店でメニューを見る ――
                vs.clear();
                vs.push_back(_T("Browsing the menu, the three debated what to order."));
                vs.push_back(_T("\"Guess we have to order this one,\""));
                vs.push_back(_T("Trapezoid Man said, showing them a photo."));
                vvs.push_back(vs);

                // ―― パエリアの話題 ――
                vs.clear();
                vs.push_back(_T("Starman and Triangle Man leaned in; the dish was paella."));
                vs.push_back(_T("\"Turns out those black shells really were mussels after all,\" Starman said."));
                vvs.push_back(vs);

                // ―― 味比べの提案 ――
                vs.clear();
                vs.push_back(_T("\"Let's compare the taste. I never liked finding shells mixed in with the rice.\""));
                vs.push_back(_T("\"How about now?\""));
                vs.push_back(_T("\"Hmm...\""));
                vvs.push_back(vs);

                // ―― 無人島の食事を思い出す ――
                vs.clear();
                vs.push_back(_T("\"We ate so many muddy things back on that island...\""));
                vs.push_back(_T("\"Yeah, I know...\""));
                vs.push_back(_T("Soon the paella, a nata-de-coco dessert, and fresh fruit arrived."));
                vvs.push_back(vs);

                // ―― フルーツばかり頼む理由 ――
                vs.clear();
                vs.push_back(_T("\"They're probably wondering why we ordered so much fruit,\" Trapezoid Man joked."));
                vs.push_back(_T("The three of them tried the mussels."));
                vvs.push_back(vs);

                // ―― 無人島のムール貝には勝てない ――
                vs.clear();
                vs.push_back(_T("\"Tastes the same... but nothing beats the mussels we had on the island,\" Triangle Man said proudly."));
                vs.push_back(_T("Starman drifted back to that day."));
                vvs.push_back(vs);

                // ―― 思い出話 ――
                vs.clear();
                vs.push_back(_T("\"We once hiked all night through the forest from the village hall to the coastal cave."));
                vs.push_back(_T("We reached it at night, built a fire, and the four of us played cards"));
                vs.push_back(_T("while eating these mussels. Good times.\""));
                vvs.push_back(vs);

                // ―― 次の約束 ――
                vs.clear();
                vs.push_back(_T("\"... Should we hit the beach next time we meet—just the three of us?\""));
                vs.push_back(_T("\"I’m in!\""));
                vvs.push_back(vs);

                // ―― 決定 ――
                vs.clear();
                vs.push_back(_T("\"...\""));
                vs.push_back(_T("They looked at one another."));
                vs.push_back(_T("\"It’s settled, then,\""));
                vvs.push_back(vs);

                // ―― ホシマンの一言 ――
                vs.clear();
                vs.push_back(_T("Starman said."));
                vvs.push_back(vs);

                page.SetTextList(vvs);
                pageList.push_back(page);
            }

            if (!SharedObj::IsEnglish())
            {
                Page page;
                ISprite* sprite = NEW NSStoryTelling::Sprite(SharedObj::GetD3DDevice());
                sprite->Load(_T("res\\image\\ending10.png"));
                page.SetSprite(sprite);
                std::vector<std::vector<std::wstring> > vvs;
                std::vector<std::wstring> vs;

                vs.push_back(_T(""));
                vvs.push_back(vs);
                page.SetTextList(vvs);
                pageList.push_back(page);
            }
            else
            {
                Page page;
                ISprite* sprite = NEW NSStoryTelling::Sprite(SharedObj::GetD3DDevice());
                sprite->Load(_T("res\\image\\ending10.en.png"));
                page.SetSprite(sprite);
                std::vector<std::vector<std::wstring> > vvs;
                std::vector<std::wstring> vs;

                vs.push_back(_T(""));
                vvs.push_back(vs);
                page.SetTextList(vvs);
                pageList.push_back(page);
            }
        }
        else
        {
            if (!SharedObj::IsEnglish())
            {
                Page page;
                ISprite* sprite = NEW NSStoryTelling::Sprite(SharedObj::GetD3DDevice());
                sprite->Load(_T("res\\image\\ending01.png"));
                page.SetSprite(sprite);
                std::vector<std::vector<std::wstring> > vvs;
                std::vector<std::wstring> vs;
                vs.push_back(_T("数時間後、ホシマンは目を覚まし砂浜から内陸の方に歩いていった。"));
                vs.push_back(_T("しばらくして道路を見つけた。"));
                vs.push_back(_T("道路を歩き続けるとガソリンスタンドがあった。"));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("そこでホシマンは今いる場所がジーフォーク共和国であることを確認した。"));
                vs.push_back(_T("北西に進んでいたつもりだったが南に相当ずれていた。"));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("ホシマンを見た通行人に通報され、すぐにホシマンは警察に保護された。"));
                vs.push_back(_T("ホシマンは警察に事情を話し、無人島へ救助隊が派遣されることになった。"));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("救助隊は生存者を一人だけ発見した。"));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("救助隊は生存者に、ほかの生存者はいないか尋ねた。"));
                vvs.push_back(vs);

                vs.clear();
                vs.push_back(_T("生存者は答えた。"));
                vs.push_back(_T("「この島で自分以外に人を見かけたことはない」"));
                vvs.push_back(vs);

                page.SetTextList(vvs);
                pageList.push_back(page);
            }
            else
            {
                Page page;
                ISprite* sprite = NEW NSStoryTelling::Sprite(SharedObj::GetD3DDevice());
                sprite->Load(_T("res\\image\\ending01.png"));
                page.SetSprite(sprite);
                std::vector<std::vector<std::wstring> > vvs;
                std::vector<std::wstring> vs;

                // ―― 砂浜から内陸へ ――
                vs.push_back(_T("Several hours later, Starman awoke and headed inland from the beach."));
                vs.push_back(_T("After a while he found a road."));
                vs.push_back(_T("Following it, he came upon a gas station."));
                vvs.push_back(vs);

                // ―― 現在地の確認 ――
                vs.clear();
                vs.push_back(_T("There he confirmed he was in the Republic of GEFORQE."));
                vs.push_back(_T("He thought he’d been walking northwest, but he had drifted far south."));
                vvs.push_back(vs);

                // ―― 通報と保護 ――
                vs.clear();
                vs.push_back(_T("A passer-by who saw him alerted the authorities, and Starman was quickly taken into protective custody."));
                vs.push_back(_T("He explained the situation to the police, and a rescue team was dispatched to the deserted island."));
                vvs.push_back(vs);

                // ―― 救助隊の発見 ――
                vs.clear();
                vs.push_back(_T("The rescuers found only one survivor."));
                vvs.push_back(vs);

                // ―― 追加の生存者確認 ――
                vs.clear();
                vs.push_back(_T("They asked the survivor if anyone else was still alive."));
                vvs.push_back(vs);

                // ―― 生存者の返答 ――
                vs.clear();
                vs.push_back(_T("The survivor answered,"));
                vs.push_back(_T("\"I've never seen anyone else on that island besides myself.\""));
                vvs.push_back(vs);

                page.SetTextList(vvs);
                pageList.push_back(page);
            }

            if (!SharedObj::IsEnglish())
            {
                Page page;
                ISprite* sprite = NEW NSStoryTelling::Sprite(SharedObj::GetD3DDevice());
                sprite->Load(_T("res\\image\\ending10.png"));
                page.SetSprite(sprite);
                std::vector<std::vector<std::wstring> > vvs;
                std::vector<std::wstring> vs;

                vs.push_back(_T(""));
                vvs.push_back(vs);
                page.SetTextList(vvs);
                pageList.push_back(page);
            }
            else
            {
                Page page;
                ISprite* sprite = NEW NSStoryTelling::Sprite(SharedObj::GetD3DDevice());
                sprite->Load(_T("res\\image\\ending10.en.png"));
                page.SetSprite(sprite);
                std::vector<std::vector<std::wstring> > vvs;
                std::vector<std::wstring> vs;

                vs.push_back(_T(""));
                vvs.push_back(vs);
                page.SetTextList(vvs);
                pageList.push_back(page);
            }
        }

        m_storyTelling = NEW StoryTelling();
        m_storyTelling->Init(pFont, pSE, sprTextBack, sprFade, pageList, SharedObj::IsEnglish());
    }
}

SeqEnding::~SeqEnding()
{
    BGMManager::Get()->SetEnding(false);
    BGMManager::Get()->SetTrueEnd(false);
}

void SeqEnding::Update(eSequence* sequence)
{
    if (m_storyTelling != nullptr)
    {
        if (!m_bFinish)
        {
            if (GamePad::IsDownFirst(eGamePadButtonType::A))
            {
                m_storyTelling->Next();
                m_firstPage = false;
            }

            if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_RETURN))
            {
                m_storyTelling->Next();
                m_firstPage = false;
            }

            if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_SPACE))
            {
                m_storyTelling->Next();
                m_firstPage = false;
            }

            if (Mouse::IsDownLeft())
            {
                m_storyTelling->Next();
                m_firstPage = false;
            }

            if (m_firstPage == false && m_bPlay == false)
            {
                m_bPlay = true;
            }

            m_bFinish = m_storyTelling->Update();
            if (m_bFinish)
            {
                if (!m_bTrueEnding)
                {
                    m_storyTelling->Finalize();
                    delete m_storyTelling;
                    m_storyTelling = nullptr;
                    *sequence = eSequence::BATTLE;
                }
                else
                {
                    BGMManager::Get()->SetEnding(false);
                    BGMManager::Get()->SetTrueEnd(true);
                    Common::SetCursorVisibility(true);
                }
            }
        }
    }
}

void SeqEnding::Render()
{
    if (m_storyTelling != nullptr)
    {
        m_storyTelling->Render();

        if (m_bFinish && m_bTrueEnding)
        {
            D3DXVECTOR3 pos(0.f, 0.f, 0.f);
            m_spriteEnd->Render(pos);
        }
    }
}
