// Kofusai Game Launcher v.2.3.5
// Version: OpenSiv3D v0.4.3
// (c) 2020 Waseda High School PC Programming Club
// Made by S.Takahashi
#include <Siv3D.hpp>
#include <Windows.h>
#include <locale.h>
#include <atlstr.h>
#include <winuser.h>
#include "HTTPClient.hpp"

#define ALOAD AssetParameter::LoadAsync()
#define FASET FontAsset::Register
#define TASET TextureAsset::Register
#define AASET AudioAsset::Register

class LGame {
public:
	FilePath fpp;
	String screenShotPath;
	String logoPath;
	String target;
	String staff;
	String grade;
	String desc;
	FilePath launcPath;
	String title;
	String app;
	LGame(FilePath fp) {
		fpp = fp;
		INIData info_ini = INIData(fpp + U"info.ini");
		screenShotPath = fpp + info_ini.get<String>(U"Game.image");
		logoPath = fpp + info_ini.get<String>(U"Game.logo");
		target = info_ini.get<String>(U"Game.target");
		staff = info_ini.get<String>(U"Game.staff");
		grade = info_ini.get<String>(U"Game.grade");
		desc = info_ini.get<String>(U"Game.desc");
		launcPath = fpp + info_ini.get<String>(U"Game.path");
		title = info_ini.get<String>(U"Game.title");
		app = info_ini.get<String>(U"Game.app");
	}
};

std::vector<std::string> split(std::string str, char del) {
	int first = 0;
	int last = str.find_first_of(del);
	std::vector<std::string> result;
	while (first < str.size()) {
		std::string subStr(str, first, last - first);
		result.push_back(subStr);
		first = last + 1;
		last = str.find_first_of(del, first);
		if (last == std::string::npos) {
			last = str.size();
		}
	}
	return result;
}

void HideTaskBar(bool hide)
{
	HANDLE hWnd = FindWindow(TEXT("Shell_TrayWnd"), NULL);
	APPBARDATA ABData;
	ABData.cbSize = sizeof(ABData);
	ABData.hWnd = (HWND)hWnd;
	if (hide)ABData.lParam = ABS_AUTOHIDE;
	if (!hide) ABData.lParam = ABS_ALWAYSONTOP;
	SHAppBarMessage(ABM_SETSTATE, &ABData);
}


enum Scenes {
	SelectPCnoScene,
	ShowLogoScene,
	StartScene,
	ErrorScene,
	DescriptionScene,
	MainMenuScene,
	SettingScene,
	GamingScene,
	GoodByeScene
};

void Main()
{
	//MainConfig
	{
		Scene::SetBackground(Palette::Black);
		//Cursor::ClipToWindow(true);
		HideTaskBar(true);
		Cursor::SetPos(0, 1080);
		Cursor::RequestStyle(CursorStyle::Hidden);
		Window::SetTitle(U"KofusaiLauncher");
		System::SetTerminationTriggers(UserAction::None);
		Window::Resize(1920, 1080);
		Window::SetFullscreen(true, Size(1920, 1080));
		Window::SetStyle(WindowStyle::Frameless);
	}
	//LoadConfigFile
	if (!FileSystem::Exists(U"launcher.ini")) {
		INIData ini;
		ini.write(U"Config", U"network", U"true");
		ini.write(U"Config", U"asketime", U"true");
		ini.write(U"Config", U"pcno", 1);
		ini.write(U"Config", U"bgm", U"true");
		ini.write(U"Config", U"fps", U"false");
		ini.write(U"Config", U"logo", U"false");
		ini.save(U"launcher.ini");
	}
	INIData inii(U"launcher.ini");
	bool enable_network = inii.getOr<bool>(U"Config.network", true);
	bool enable_asketime = inii.getOr<bool>(U"Config.asketime", true);
	bool enable_bgm = inii.getOr<bool>(U"Config.bgm", true);
	bool enable_fps = inii.getOr<bool>(U"Config.fps", false);
	bool enable_logo = inii.getOr<bool>(U"Config.logo", false);
	int pcno = inii.getOr<int>(U"Config.pcno", 1);
	//SceneSetting
	Scenes scene;
	if (enable_logo) {
		scene = ShowLogoScene;
	}
	else {
		if (enable_network) {
			if (enable_asketime) {
				scene = SelectPCnoScene;
			}
			else {
				if (pcno == NULL) {
					scene = SelectPCnoScene;
				}
				else {
					scene = StartScene;
				}
			}
		}
		else {
			scene = StartScene;
		}
	}
	//MainAssets
	Array<String> FAssetNames;
	Array<String> AAssetNames;
	Array<String> TAssetNames;
	//-ShowLogoScene
	FASET(U"Smart30", 30, U"Assets/Font/03SmartFontUI.ttf", ALOAD);
	TASET(U"logo", U"Assets/logo.png", ALOAD);
	FAssetNames.push_back(U"Smart30");
	TAssetNames.push_back(U"logo");
	//-StartScene
	FASET(U"Smart70", 70, U"Assets/Font/03SmartFontUI.ttf", ALOAD);
	TASET(U"back", U"Assets/back.png");
	TASET(U"backg", U"Assets/background.png");
	FAssetNames.push_back(U"Smart70");
	TAssetNames.push_back(U"back");
	TAssetNames.push_back(U"backg");
	//-SelectPcnoScene
	FASET(U"Smart50", 50, U"Assets/Font/03SmartFontUI.ttf", ALOAD);
	FAssetNames.push_back(U"Smart50");
	//-DescriptionScene
	FASET(U"Smart65", 65, U"Assets/Font/03SmartFontUI.ttf", ALOAD);
	FASET(U"Smart255", 255, U"Assets/Font/03SmartFontUI.ttf", ALOAD);
	TASET(U"howto", U"Assets/howto.png", ALOAD);
	FAssetNames.push_back(U"Smart65");
	FAssetNames.push_back(U"Smart255");
	TAssetNames.push_back(U"howto");
	//-MainMenuScene
	FASET(U"Smart60", 60, U"Assets/Font/03SmartFontUI.ttf", ALOAD);
	FAssetNames.push_back(U"Smart60");
	TASET(U"haguruma", U"Assets/haguruma.png");
	TAssetNames.push_back(U"haguruma");
	///-LoadGames
	int gamenumcounter = 0;
	std::vector<LGame> games;
	for (const FilePath& fp : FileSystem::DirectoryContents(U"./Games", false)) {
		LGame lg = LGame(fp);
		games.push_back(lg);
		gamenumcounter++;
		TASET(s3d::Format(U"GameSS", gamenumcounter), lg.screenShotPath, ALOAD);
		TAssetNames.push_back(s3d::Format(U"GameSS", gamenumcounter));
		TASET(s3d::Format(U"GameLogo", gamenumcounter), lg.logoPath, ALOAD);
		TAssetNames.push_back(s3d::Format(U"GameLogo", gamenumcounter));
	}
	///-LoadAudios
	JSONReader music_info(U"Music/info.json");
	int musicnumcounter = 0;
	std::vector<String> musicNames;
	for (const auto& o : music_info[U"Musics"].arrayView()) {
		String mpath = o[U"Path"].get<String>();
		musicNames.push_back(o[U"Name"].get<String>());
		musicnumcounter++;
		AASET(s3d::Format(U"BGM", musicnumcounter), mpath, ALOAD);
		AAssetNames.push_back(s3d::Format(U"BGM", musicnumcounter));
	}
	//checkServerIP
	IPv4 serverip;
	if (enable_network) {
		if (!HTTPClient::InitCURL()) return;
		HTTPClient httpclient;
		FilePath resultFilePath(U"result.data");
		if (!httpclient.downloadFile(U"https://negima-p.work/api/v1/pcp/getip.php", resultFilePath)) return;
		String ipaddress = TextReader(resultFilePath).readAll();
		std::vector<std::string> ipparse = split(ipaddress.narrow(), '.');
		serverip = { static_cast<s3d::uint8>(std::stoi(ipparse[0])), static_cast<s3d::uint8>(std::stoi(ipparse[1])), static_cast<s3d::uint8>(std::stoi(ipparse[2])), static_cast<s3d::uint8>(std::stoi(ipparse[3])) };
	}
	//Main
	if (scene == ShowLogoScene) {
		int logo_alpha = 0;
		int su_status = 0;
		while (System::Update()) {
			TextureAsset(U"backg").resized(1920, 1080).draw(0, 0);
			if ((KeyEscape.pressed() && KeyX.pressed() && KeyV.pressed()) || System::GetUserActions() == UserAction::CloseButtonClicked) {
				HideTaskBar(false);
				System::Exit();
			}
			if (enable_fps) {
				FontAsset(U"Smart30")(Profiler::FPS(), U"fps").draw(1810, 1020, Palette::White);
			}
			if (su_status == 0) {
				logo_alpha += 3;
				FontAsset(U"Smart60")(U"アセットを読み込み中です...").drawAt(960, 800, Color(Palette::White, logo_alpha));
			}
			if (su_status == 2) {
				logo_alpha -= 3;
				FontAsset(U"Smart60")(U"読み込み完了...").drawAt(960, 800, Color(Palette::White, logo_alpha));
			}
			if (logo_alpha >= 255) {
				su_status = 1;
				logo_alpha = 255;
			}
			if (su_status == 1) {
				bool AllAssetsReady = false;
				for (auto i : FAssetNames) {
					AllAssetsReady = FontAsset::IsReady(i);
					Print(U"Load:", i, Palette::Green);
					if (!AllAssetsReady) break;
				}
				if (AllAssetsReady) {
					for (auto i : TAssetNames) {
						AllAssetsReady = TextureAsset::IsReady(i);
						Print(U"Load:", i, Palette::Blue);
						if (!AllAssetsReady) break;
					}
				}
				if (AllAssetsReady) {
					for (auto i : AAssetNames) {
						AllAssetsReady = AudioAsset::IsReady(i);
						Print(U"Load:", i, Palette::Red);
						if (!AllAssetsReady) break;
					}
				}
				Print(U"Load:", AllAssetsReady, Palette::White);
				if (AllAssetsReady) {
					su_status = 2;
				}
				FontAsset(U"Smart60")(U"アセットを読み込み中です...").drawAt(960, 800, Color(Palette::White, logo_alpha));
			}
			if (logo_alpha < 0) {
				if (enable_network) {
					if (enable_asketime) {
						scene = SelectPCnoScene;
					}
					else {
						if (pcno == NULL) {
							scene = SelectPCnoScene;
						}
						else {
							scene = StartScene;
						}
					}
				}
				else {
					scene = StartScene;
				}
				ClearPrint();
				break;
			}
			TextureAsset(U"logo").scaled(0.3).drawAt(960, 540, Color(255, 255, 255, logo_alpha));
		}
	}
	if (scene == SelectPCnoScene) {
		String pcno_s;
		String errormes;
		while (System::Update()) {
			TextureAsset(U"backg").resized(1920, 1080).draw(0, 0);
			if ((KeyEscape.pressed() && KeyX.pressed() && KeyV.pressed()) || System::GetUserActions() == UserAction::CloseButtonClicked) {
				HideTaskBar(false);
				System::Exit();
			}
			if (enable_fps) {
				FontAsset(U"Smart30")(Profiler::FPS(), U"fps").draw(1810, 1020, Palette::White);
			}
			FontAsset(U"Smart65")(U"PCの番号を入力してください。").drawAt(960, 350);
			if (pcno_s.size() <= 1) {
				if (Key1.down()) pcno_s.push_back('1');
				if (Key2.down()) pcno_s.push_back('2');
				if (Key3.down()) pcno_s.push_back('3');
				if (Key4.down()) pcno_s.push_back('4');
				if (Key5.down()) pcno_s.push_back('5');
				if (Key6.down()) pcno_s.push_back('6');
				if (Key7.down()) pcno_s.push_back('7');
				if (Key8.down()) pcno_s.push_back('8');
				if (Key9.down()) pcno_s.push_back('9');
				if (Key0.down()) pcno_s.push_back('0');
			}
			if (pcno_s.size() <= 0) {
				if (Key0.down()) pcno_s.push_back('0');
			}
			if (KeyBackspace.down()) {
				if (pcno_s.size() >= 1) {
					pcno_s.pop_back();
				}
			}
			if (KeyEnter.down() || KeyZ.down()) {
				if (!pcno_s.length() > 0) {
					errormes = U"1から12で入力してください";
					pcno_s.clear();
				}
				else {
					if (pcno_s[0] == '0')pcno_s.pop_front();
					int temp_pcno_s = Parse<int>(pcno_s);
					if (temp_pcno_s > 0 && temp_pcno_s <= 12) {
						pcno = temp_pcno_s;
						INIData initemp;
						initemp.write(U"Config", U"network", enable_network);
						initemp.write(U"Config", U"asketime", enable_asketime);
						initemp.write(U"Config", U"pcno", temp_pcno_s);
						initemp.write(U"Config", U"bgm", enable_bgm);
						initemp.write(U"Config", U"fps", enable_fps);
						initemp.write(U"Config", U"logo", enable_logo);
						initemp.save(U"launcher.ini");
						scene = StartScene;
						break;
					}
					else {
						errormes = U"不適切な数値です。1から12で入力してください";
						pcno_s.clear();
					}
				}
			}
			FontAsset(U"Smart255")(pcno_s).draw(800, 400);
			FontAsset(U"Smart65")(errormes).drawAt(960, 800, Palette::Red);
		}
	}
start:
	//TCP
	TCPClient client;
	std::string message;
	bool connected = false;
	bool isFirst = true;
	if (enable_network) {
		if (client.isConnected()) client.disconnect();
		client.connect(serverip, 51126);
	}
	SetCursorPos(1920, 0);
start2:
	if (scene == StartScene) {
		String errormes2 = U"接続試行中...";
		NetworkError er;
		String mainmes = U"Press the 〇 button to start";
		String submes = U"〇ボタンを押してスタート";
		const int32 cycle = 1500;
		double back_scale = 1;
		int selecter = 0;
		uint32 rchi = 1;
		int textcount = 0;
		int textselect = 0;
		while (System::Update()) {
			TextureAsset(U"backg").resized(1920, 1080).draw(0, 0);
			if ((KeyEscape.pressed() && KeyX.pressed() && KeyV.pressed()) || System::GetUserActions() == UserAction::CloseButtonClicked) {
				HideTaskBar(false);
				System::Exit();
			}
			if (enable_fps) {
				FontAsset(U"Smart30")(Profiler::FPS(), U"fps").draw(1810, 1020, Palette::White);
			}
			if (enable_network) {
				er = client.getError();
				if (client.isConnected()) {
					if (!connected) {
						//接続時はじめに
						connected = true;
						errormes2 = U"";
						const String sss = s3d::Format(U"Rgs:", pcno, U"\n");
						const std::string sendsss = sss.toUTF8();
						client.send(sendsss.data(), sizeof(char) * sendsss.length());
					}
					//TCP取得処理
					message.clear();
					for (;;) {
						char ch;
						if (!client.read(ch)) {
							break;
						}
						if (ch == '\n') {
							break;
						}
						else {
							message.push_back(ch);
						}
					}
				}
				if (client.hasError()) {
					errormes2 = U"TCPサーバーに接続できません。";
					switch (er) {
					case NetworkError::ConnectionRefused:
						errormes2 = U"TCPサーバーに接続できません。Err:ConnectionRefused";
						break;
					case NetworkError::EoF:
						errormes2 = U"TCPサーバーに接続できません。Err:EoF";
						break;
					case NetworkError::Error:

						errormes2 = U"TCPサーバーに接続できません。Err:Error";
						break;
					case NetworkError::NoBufferSpaceAvailable:
						errormes2 = U"TCPサーバーに接続できません。Err:NoBufferSpaceAvailable";
						break;
					case NetworkError::OK:
						errormes2 = U"TCPサーバーに接続できません。Err:OK?";
						break;
					}
					connected = false;
					client.disconnect();
					client.connect(serverip, 51126);
					continue;
				}
				if (connected) {
					const int32 t = Time::GetMillisec();
					const double a1 = (cycle - t % cycle) / static_cast<double>(cycle);
					const ColorF bb = ColorF(1, rchi, rchi, a1);
					if (textselect == 0) {
						FontAsset(U"Smart70")(mainmes).drawAt(960, 900, bb);
						if (a1 * 100 <= 1) {
							textcount++;
							if (textcount == 2) {
								textselect = 1;
								textcount = 0;
							}
						}
					}
					else {
						FontAsset(U"Smart70")(submes).drawAt(960, 900, bb);
						if (a1 * 100 <= 1) {
							textcount++;
							if (textcount == 2) {
								textselect = 0;
								textcount = 0;
							}
						}
					}
					if (KeyZ.down()) {
						scene = DescriptionScene;
						break;
					}
				}
				else {
					FontAsset(U"Smart70")(errormes2).drawAt(960, 900, Palette::Red);
				}
			}
			else {
				const int32 t = Time::GetMillisec();
				const double a1 = (cycle - t % cycle) / static_cast<double>(cycle);
				const ColorF bb = ColorF(1, rchi, rchi, a1);
				if (textselect == 0) {
					FontAsset(U"Smart70")(mainmes).drawAt(960, 900, bb);
					if (a1 * 100 <= 1) {
						textcount++;
						if (textcount == 2) {
							textselect = 1;
							textcount = 0;
						}
					}
				}
				else {
					FontAsset(U"Smart70")(submes).drawAt(960, 900, bb);
					if (a1 * 100 <= 1) {
						textcount++;
						if (textcount == 2) {
							textselect = 0;
							textcount = 0;
						}
					}
				}
				if (KeyZ.down()) {
					scene = DescriptionScene;
					break;
				}
			}
			//回ってるやつ
			if (selecter == 0) {
				if (back_scale <= -1.00) {
					selecter = 1;
				}
				back_scale -= 0.02;
			}
			else {
				if (back_scale >= 1.00) {
					selecter = 0;
				}
				back_scale += 0.02;
			}
			TextureAsset(U"back").scaled(0.35).scaled(back_scale, 1).drawAt(960, 500);
		}
	}
	struct RingEffect : IEffect
	{
		Vec2 m_pos;
		// このコンストラクタ引数が、Effect::add<RingEffect>() の引数になる
		RingEffect(const Vec2& pos)
			: m_pos(pos) {}
		bool update(double t) override
		{
			// 時間に応じて大きくなる輪
			Circle(m_pos, 5+(t * 60)).draw(Color(Palette::Yellow, 120+(t*20)));
			// 1 秒以上経過したら消滅
			return t < 0.25;
		}
	};
	if (scene == DescriptionScene) {
		int limit = 300;
		isFirst = false;
		bool getLimitYet = false;
		String mesagee = U"制限時間を取得中です";
		Effect effect;
		while (System::Update()) {
			TextureAsset(U"backg").resized(1920, 1080).draw(0, 0);
			if ((KeyEscape.pressed() && KeyX.pressed() && KeyV.pressed()) || System::GetUserActions() == UserAction::CloseButtonClicked) {
				HideTaskBar(false);
				System::Exit();
			}
			if (enable_fps) {
				FontAsset(U"Smart30")(Profiler::FPS(), U"fps").draw(1810, 1020, Palette::White);
			}
			FontAsset(U"Smart65")(U"制限時間は").draw(250, 50);
			String minutestr = s3d::Format(limit % 60);
			if (minutestr.length() <= 1) minutestr.push_front('0');
			FontAsset(U"Smart255")((limit / 60)).draw(830 - FontAsset(U"Smart255")((limit / 60)).region(700, 90).size.x, 90); FontAsset(U"Smart65")(U"分").draw(845, 250); FontAsset(U"Smart255")(minutestr).draw(920, 90); FontAsset(U"Smart65")(U"秒 です").draw(1250, 250);
			FontAsset(U"Smart65")(U"※混雑状況によって変更されます").drawAt(960, 410);
			TextureAsset(U"howto").drawAt(960, 740);
			FontAsset(U"Smart65")(mesagee).drawAt(960, 980);
			if (KeyB.down()) {
				scene = StartScene;
				goto start;
				break;
			}
			if (client.hasError()) {
				connected = false;
				client.disconnect();
			}
			if (enable_network) {
				if (connected) {
					//TCP取得処理
					message.clear();
					for (;;) {
						char ch;
						if (!client.read(ch)) {
							break;
						}
						if (ch == '\n') {
							break;
						}
						else {
							message.push_back(ch);
						}
					}
					if (message != "") {
						String mes = Unicode::FromUTF8(message);
						String command = mes.substr(0, 3);
						String text = mes.substr(4);
						if (command == U"Lmt") {
							limit = ParseOr<int>(text, 0);
							getLimitYet = true;
							mesagee = U"〇：スタート";
						}
					}
					if (KeyZ.down() && getLimitYet) {
						scene = MainMenuScene;
						break;
					}
				}
				else {
					scene = StartScene;
					goto start;
					break;
				}
			}
			else {
				mesagee = U"〇：スタート";
				if (KeyZ.down()) {
					scene = MainMenuScene;
					break;
				}
			}
			if (KeyRight.down())effect.add<RingEffect>(Vec2(613, 680));
			if (KeyLeft.down())effect.add<RingEffect>(Vec2(563, 680));
			if (KeyUp.down())effect.add<RingEffect>(Vec2(588, 660));
			if (KeyDown.down())effect.add<RingEffect>(Vec2(588, 710));
			if (KeyZ.down())effect.add<RingEffect>(Vec2(855, 680));
			if (KeyX.down())effect.add<RingEffect>(Vec2(805, 680));
			if (KeyC.down())effect.add<RingEffect>(Vec2(840, 650));
			if (KeyV.down())effect.add<RingEffect>(Vec2(840, 720));
			if (KeyEscape.down())effect.add<RingEffect>(Vec2(685, 675));
			if (KeyEnter.down())effect.add<RingEffect>(Vec2(745, 675));
			if (KeyD.down())effect.add<RingEffect>(Vec2(1198, 710));
			if (KeyA.down())effect.add<RingEffect>(Vec2(1433, 710));
			if (KeyW.down())effect.add<RingEffect>(Vec2(1198, 760));
			if (KeyS.down())effect.add<RingEffect>(Vec2(1433, 760));
			effect.update();
		}
	}
	//Process
	Optional<ChildProcess> process;
	Optional<ChildProcess> NotifPros;
	HWND hWnd = FindWindow(NULL, TEXT("KofusaiLauncher"));
	if (hWnd == NULL) System::Exit();
main:
	Cursor::RequestStyle(CursorStyle::Hidden);
	int Rectselecter = 0;
	if (scene == MainMenuScene) {
		//TODO TCP関係の処理
		const int32 cycle = 1200;
		double FirstPoint = 540 - ((double)games.size() / 2) * 20;
		Array<Rect> selectRect;
		for (int i = 0; i < games.size(); i++) {
			selectRect.push_back(Rect(25, (double)(FirstPoint + i * 20), 10, 10));
		}
		//---BGM
		const s3d::Polygon detekuru{
			{0, 0}, {900, 0},{850, 80},{0, 80}
		};
		Rect desc_way(950, 550, 970, 220);
		//KOBETSUKOBETSUKOBETSUKOBETSUKOBETSUKOBETSUKOBETSU
		int selecter = 0;
		double scall = (double)500 / TextureAsset(U"GameSS1").height();
		String target = games[0].target;
		String grade = games[0].grade;
		String staff = games[0].staff;
		FilePath launcPath = games[0].launcPath;
		String title = games[0].title;
		String desc = games[0].desc;
		Array<String> descText;
		int desccount = 0;
		String desctemp;
		for (wchar_t o : desc) {
			desccount++;
			desctemp.append(o);
			if (desccount >= 15) {
				desccount = 0;
				descText.push_back(desctemp);
				desctemp.clear();
			}
		}
		desccount = 0;
		descText.push_back(desctemp);
		desctemp.clear();
		//TCP
		if (enable_network) {
			if (connected) {
				const String Str = s3d::Format(U"Srt:", pcno, U"\n");
				const std::string str = Str.toUTF8();
				client.send(str.data(), sizeof(char) * str.length());
			}
			else {
				scene = StartScene;
				goto start;
			}
		}
		int num_games = games.size();
		//anim
		Array<Vec2> logosPoint = {
			Point(310, -190), Point(350, 50), Point(390, 290), Point(430, 540),
			Point(470, 790), Point(510, 1030), Point(550, 1270)
		};
		Vec2 scaleBegin = Vec2(0.8, 0);
		Vec2 scaleEnd = Vec2(1, 0);
		Rectselecter = 0;
		Stopwatch st01;
		Stopwatch st02;
		st02.start();
		int selecterMode = 0;//0:None 1:Down 2:Up
		constexpr Vec2 begin(0, 50);
		constexpr Vec2 end(0, 0);
		while (System::Update()) {
			TextureAsset(U"backg").resized(1920, 1080).draw(0, 0);
			//Receive TCP message
			if (enable_network) {
				std::string tmessage = "";
				while (true) {
					char ch;
					if (!client.read(ch)) break;
					if (ch == '\n') break;
					else tmessage.push_back(ch);
				}
				if (tmessage != "") {
					String mes = Unicode::FromUTF8(tmessage);
					String command = mes.substr(0, 3);
					String text = mes.substr(4);
					if (command == U"Atn") {
						if (text == U"Rcz") {
							NotifPros = Process::Spawn(U"./onTime.exe");
							scene = Scenes::GoodByeScene;
							break;
						}
					}
				}
			}
			//
			if ((KeyEscape.pressed() && KeyX.pressed() && KeyV.pressed()) || System::GetUserActions() == UserAction::CloseButtonClicked) {
				HideTaskBar(false);
				System::Exit();
			}
			TextureAsset(U"haguruma").resized(350, 350).rotated(Scene::Time() * 30_deg).drawAt(1850, 1010);
			if (enable_fps) {
				FontAsset(U"Smart30")(Profiler::FPS(), U"fps").draw(1810, 1020, Palette::White);
			}
			const double t = Min(st01.sF(), 0.6);
			const double pe = EaseOutExpo(t);
			if (t == 0 && KeyDown.down() && selecter < (num_games - 1)) {
				st01.start();
				selecterMode = 1;
				Rectselecter++;
				st02.restart();
				descText.clear();
				desccount = 0;
				desctemp.clear();
				for (wchar_t o : games[Rectselecter].desc) {
					desccount++;
					desctemp.append(o);
					if (desccount >= 15) {
						desccount = 0;
						descText.push_back(desctemp);
						desctemp.clear();
					}
				}
				desccount = 0;
				descText.push_back(desctemp);
				desctemp.clear();
			}
			else if (t == 0 && KeyUp.down() && selecter > 0) {
				st01.start();
				selecterMode = 2;
				Rectselecter--;
				st02.restart();
				descText.clear();
				desccount = 0;
				desctemp.clear();
				for (wchar_t o : games[Rectselecter].desc) {
					desccount++;
					desctemp.append(o);
					if (desccount >= 15) {
						desccount = 0;
						descText.push_back(desctemp);
						desctemp.clear();
					}
				}
				desccount = 0;
				descText.push_back(desctemp);
				desctemp.clear();
			}
			else if (t == 0 && KeyUp.down() && selecter == 0) {
				st01.start();
				selecter = (num_games - 2);
				selecterMode = 1;
				Rectselecter = (num_games - 1);
				st02.restart();
				descText.clear();
				desccount = 0;
				desctemp.clear();
				for (wchar_t o : games[Rectselecter].desc) {
					desccount++;
					desctemp.append(o);
					if (desccount >= 15) {
						desccount = 0;
						descText.push_back(desctemp);
						desctemp.clear();
					}
				}
				desccount = 0;
				descText.push_back(desctemp);
				desctemp.clear();
			}
			else if (t == 0 && KeyDown.down() && selecter == (num_games - 1)) {
				st01.start();
				selecter = 1;
				selecterMode = 2;
				Rectselecter = 0;
				st02.restart();
				descText.clear();
				desccount = 0;
				desctemp.clear();
				for (wchar_t o : games[Rectselecter].desc) {
					desccount++;
					desctemp.append(o);
					if (desccount >= 15) {
						desccount = 0;
						descText.push_back(desctemp);
						desctemp.clear();
					}
				}
				desccount = 0;
				descText.push_back(desctemp);
				desctemp.clear();
			}
			if (selecter < 0 || selecter >(num_games - 1)) {
				selecter = 0;
				st01.reset();
				selecterMode = 0;
			}
			if (t >= 0.6) {
				st01.reset();
				if (selecterMode == 1 && selecter < (num_games - 1)) selecter++;
				else if (selecterMode == 2 && selecter > 0) selecter--;
				selecterMode = 0;
			}
			if (selecterMode == 0) {
				if (selecter - 3 >= 0)TextureAsset(s3d::Format(U"GameLogo", selecter - 3 + 1)).scaled(0.8).drawAt(logosPoint[0].x, logosPoint[0].y);
				if (selecter - 2 >= 0)TextureAsset(s3d::Format(U"GameLogo", selecter - 2 + 1)).scaled(0.8).drawAt(logosPoint[1].x, logosPoint[1].y);
				if (selecter - 1 >= 0)TextureAsset(s3d::Format(U"GameLogo", selecter - 1 + 1)).scaled(0.8).drawAt(logosPoint[2].x, logosPoint[2].y);
				TextureAsset(s3d::Format(U"GameLogo", selecter + 1)).drawAt(logosPoint[3].x, logosPoint[3].y);
				if (selecter + 1 <= (num_games - 1))TextureAsset(s3d::Format(U"GameLogo", selecter + 1 + 1)).scaled(0.8).drawAt(logosPoint[4].x, logosPoint[4].y);
				if (selecter + 2 <= (num_games - 1))TextureAsset(s3d::Format(U"GameLogo", selecter + 2 + 1)).scaled(0.8).drawAt(logosPoint[5].x, logosPoint[5].y);
				if (selecter + 3 <= (num_games - 1))TextureAsset(s3d::Format(U"GameLogo", selecter + 3 + 1)).scaled(0.8).drawAt(logosPoint[6].x, logosPoint[6].y);
			}
			else if (selecterMode == 1) {
				if (selecter - 2 >= 0)TextureAsset(s3d::Format(U"GameLogo", selecter - 2 + 1)).scaled(0.8).drawAt(logosPoint[1].lerp(logosPoint[0], pe));
				if (selecter - 1 >= 0)TextureAsset(s3d::Format(U"GameLogo", selecter - 1 + 1)).scaled(0.8).drawAt(logosPoint[2].lerp(logosPoint[1], pe));
				TextureAsset(s3d::Format(U"GameLogo", selecter + 1)).scaled(scaleEnd.lerp(scaleBegin, pe).x).drawAt(logosPoint[3].lerp(logosPoint[2], pe));
				if (selecter + 1 <= (num_games - 1))TextureAsset(s3d::Format(U"GameLogo", selecter + 1 + 1)).scaled(scaleBegin.lerp(scaleEnd, pe).x).drawAt(logosPoint[4].lerp(logosPoint[3], pe));
				if (selecter + 2 <= (num_games - 1))TextureAsset(s3d::Format(U"GameLogo", selecter + 2 + 1)).scaled(0.8).drawAt(logosPoint[5].lerp(logosPoint[4], pe));
				if (selecter + 3 <= (num_games - 1))TextureAsset(s3d::Format(U"GameLogo", selecter + 3 + 1)).scaled(0.8).drawAt(logosPoint[6].lerp(logosPoint[5], pe));
			}
			else if (selecterMode == 2) {
				if (selecter - 3 >= 0)TextureAsset(s3d::Format(U"GameLogo", selecter - 3 + 1)).scaled(0.8).drawAt(logosPoint[0].lerp(logosPoint[1], pe));
				if (selecter - 2 >= 0)TextureAsset(s3d::Format(U"GameLogo", selecter - 2 + 1)).scaled(0.8).drawAt(logosPoint[1].lerp(logosPoint[2], pe));
				if (selecter - 1 >= 0)TextureAsset(s3d::Format(U"GameLogo", selecter - 1 + 1)).scaled(scaleBegin.lerp(scaleEnd, pe).x).drawAt(logosPoint[2].lerp(logosPoint[3], pe));
				TextureAsset(s3d::Format(U"GameLogo", selecter + 1)).scaled(scaleEnd.lerp(scaleBegin, pe).x).drawAt(logosPoint[3].lerp(logosPoint[4], pe));
				if (selecter + 1 <= (num_games - 1))TextureAsset(s3d::Format(U"GameLogo", selecter + 1 + 1)).scaled(0.8).drawAt(logosPoint[4].lerp(logosPoint[5], pe));
				if (selecter + 2 <= (num_games - 1))TextureAsset(s3d::Format(U"GameLogo", selecter + 2 + 1)).scaled(0.8).drawAt(logosPoint[5].lerp(logosPoint[6], pe));
			}
			const double t2 = Min(st02.sF(), 0.6);
			const double e = EaseOutQuart(t2);
			const Vec2 pos = begin.lerp(end, e);
			const int32 t23 = Time::GetMillisec();
			const double a1 = (cycle - t23 % cycle) / static_cast<double>(cycle);
			TextureAsset(s3d::Format(U"GameSS", Rectselecter + 1)).scaled((double)500 / TextureAsset(s3d::Format(U"GameSS", Rectselecter + 1)).height()).drawAt(1420, 285 + pos.y, ColorF(Palette::White, e));
			FontAsset(U"Smart60")(U"対象年齢: " + games[Rectselecter].target).draw(950, 810 + pos.y, ColorF(Palette::White, e));
			FontAsset(U"Smart60")(U"製作者: " + games[Rectselecter].grade + U" " + games[Rectselecter].staff).draw(950, 875 + pos.y, ColorF(Palette::White, e));
			FontAsset(U"Smart70")(U"〇ボタンでスタート").drawAt(1420, 990 + pos.y, ColorF(Palette::White, a1 + (1 - e)));
			for (int i = 0; i < descText.size(); i++) {
				const int32 y = static_cast<int32>(desc_way.y + 10 + FontAsset(U"Smart60").height() * i);
				FontAsset(U"Smart60")(descText[i]).draw(desc_way.x + 10, y + pos.y, ColorF(Palette::White, e));
			}
			for (int i = 0; i < num_games; i++) {
				if (i == Rectselecter) {
					selectRect[i].drawFrame(1, 0, Palette::White);
				}
				else {
					selectRect[i].draw(Palette::White);
				}
			}
			if (KeyZ.down()) {
				SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, (SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW));
				HideTaskBar(true);
				process = Process::Spawn(games[Rectselecter].launcPath);
				scene = Scenes::GamingScene;
				break;
			}
			if (KeyB.down()) {
				scene = Scenes::StartScene;
				goto start;
				break;
			}
		}
	}
	if (scene == GamingScene) {
		CString gametitlebar(games[Rectselecter].app.narrow().c_str());
		bool fffflug = false;
		HideTaskBar(true);
		HWND ghWnd = FindWindow(NULL, gametitlebar);
		//Print << (CT2CA)(gametitlebar);
		if (enable_network) {
			if (!connected) {
				scene = StartScene;
				goto start;
			}
			else {
				const String sss = s3d::Format(U"Ply:", games[Rectselecter].title, U"\n");
				const std::string sendsss = sss.toUTF8();
				client.send(sendsss.data(), sizeof(char)* sendsss.length());
			}
		}
		while (System::Update()) {
			TextureAsset(U"backg").resized(1920, 1080).draw(0, 0);
			FontAsset(U"Smart65")(U"しばらくお待ちください。").drawAt(960, 540, Palette::White);
			if (enable_fps) {
				FontAsset(U"Smart30")(Profiler::FPS(), U"fps").draw(1810, 1020, Palette::White);
			}
			//Receive TCP message
			if (enable_network) {
				std::string tmessage = "";
				while (true) {
					char ch;
					if (!client.read(ch)) break;
					if (ch == '\n') break;
					else tmessage.push_back(ch);
				}
				if (tmessage != "") {
					String mes = Unicode::FromUTF8(tmessage);
					String command = mes.substr(0, 3);
					String text = mes.substr(4);
					if (command == U"Atn") {
						if (text == U"Rcz") {
							NotifPros = Process::Spawn(U"./onTime.exe");
							scene = Scenes::GoodByeScene;
							break;
						}
					}
				}
			}
			//
			if (process) {
				if (process->isRunning())
				{
					if (ghWnd != FindWindow(NULL, gametitlebar)) {
						ghWnd = FindWindow(NULL, gametitlebar);
						SetWindowPos(ghWnd, HWND_TOPMOST, 0, 0, 0, 0, (SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW));
						//fffflug = true;
					}
					if (!fffflug) {
						//SetCursorPos(1920, 0);
						//if (GetTopWindow(NULL) != ghWnd) {
							//SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, (SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW));
							//SetWindowPos(ghWnd, HWND_TOPMOST, 0, 0, 0, 0, (SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW));
						//}
						if (ghWnd != NULL)fffflug = true;
					}
					if (GetForegroundWindow() != ghWnd) {
						SetForegroundWindow(ghWnd);
					}
					//Print << ghWnd << U":"<<hWnd <<U":"<< GetForegroundWindow();
				}
				if (!process->isRunning()) {
					Window::Restore();
					process = none;
					NotifPros = none;
					scene = Scenes::MainMenuScene;
					SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, (SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW));
					SetActiveWindow(hWnd);
					SetForegroundWindow(hWnd);
					HideTaskBar(false);
					goto main;
					break;
				}
			}
			else {
				scene = Scenes::MainMenuScene;
				goto main;
				break;
			}
		}
	}
	if (scene == Scenes::GoodByeScene) {
		Stopwatch xxx;
		xxx.start();
		while (System::Update()) {
			TextureAsset(U"backg").resized(1920, 1080).draw(0, 0);
			if ((KeyEscape.pressed() && KeyX.pressed() && KeyV.pressed()) || System::GetUserActions() == UserAction::CloseButtonClicked) {
				HideTaskBar(false);
				System::Exit();
			}
			if (enable_fps) {
				FontAsset(U"Smart30")(Profiler::FPS(), U"fps").draw(1810, 1020, Palette::White);
			}
			FontAsset(U"Smart60")(U"終了時間になりました。\n忘れ物のないようお帰りください。\nまたのご利用をおまちしていますm(__)m").drawAt(960, 540, Palette::White);
			if (KeyB.down()) {
				const String Str = s3d::Format(U"End:", pcno, U"\n");
				const std::string str = Str.toUTF8();
				client.send(str.data(), sizeof(char)* str.length());
				scene = Scenes::StartScene;
				goto start;
				break;
			}
			if (xxx.s() >= 15) {
				const String Str = s3d::Format(U"End:", pcno, U"\n");
				const std::string str = Str.toUTF8();
				client.send(str.data(), sizeof(char)* str.length());
				scene = Scenes::StartScene;
				goto start;
				break;
			}
		}
	}
}
