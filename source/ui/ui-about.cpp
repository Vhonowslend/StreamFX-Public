/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2020 Michael Fabian Dirks
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "ui-about.hpp"
#include <obs-frontend-api.h>
#include <map>
#include <random>
#include "ui-about-entry.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251 4365 4371 4619 4946)
#endif
#include <QLayout>
#include <QLayoutItem>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

constexpr std::string_view text_social_facebook = "Facebook";
constexpr std::string_view text_social_twitch   = "Twitch";
constexpr std::string_view text_social_twitter  = "Twitter";
constexpr std::string_view text_social_youtube  = "YouTube";

static const std::vector<std::string_view> _thankyous = {
	":/thankyou/thankyou_cat",
	":/thankyou/thankyou_otter",
	":/thankyou/thankyou_fox",
};

static const streamfx::ui::about::entry _entries[] = {
	// Contributers (Updated on 2021-04-23)
	// - 2021
	streamfx::ui::about::entry{"Michael \"Xaymar\" Dirks", streamfx::ui::about::role_type::CONTRIBUTOR, "",
							   "https://xaymar.com"},
	streamfx::ui::about::entry{"cpyarger", streamfx::ui::about::role_type::CONTRIBUTOR, "",
							   "https://github.com/cpyarger"},
	streamfx::ui::about::entry{"tyten652", streamfx::ui::about::role_type::CONTRIBUTOR, "",
							   "https://github.com/tytan652"},
	streamfx::ui::about::entry{"kilinbox", streamfx::ui::about::role_type::CONTRIBUTOR, "",
							   "https://github.com/kilinbox"},
	// - 2020
	streamfx::ui::about::entry{"Oncorporation", streamfx::ui::about::role_type::CONTRIBUTOR, "",
							   "https://github.com/Oncorporation"},
	streamfx::ui::about::entry{"dghodgson", streamfx::ui::about::role_type::CONTRIBUTOR, "",
							   "https://github.com/dghodgson"},
	streamfx::ui::about::entry{"danimo", streamfx::ui::about::role_type::CONTRIBUTOR, "", "https://github.com/danimo"},
	streamfx::ui::about::entry{"brandonedens", streamfx::ui::about::role_type::CONTRIBUTOR, "",
							   "https://github.com/brandonedens"},
	streamfx::ui::about::entry{"rjmoggach", streamfx::ui::about::role_type::CONTRIBUTOR, "",
							   "https://github.com/rjmoggach"},
	// - 2019
	streamfx::ui::about::entry{"catb0t", streamfx::ui::about::role_type::CONTRIBUTOR, "", "https://github.com/catb0t"},
	streamfx::ui::about::entry{"Vainock", streamfx::ui::about::role_type::CONTRIBUTOR, "",
							   "https://github.com/Vainock"},
	streamfx::ui::about::entry{"wwj402", streamfx::ui::about::role_type::CONTRIBUTOR, "", "https://github.com/wwj402"},
	// - 2018

	// Supporters (Updated on 2021-04-23)
	streamfx::ui::about::entry{"dangerbeard", streamfx::ui::about::role_type::SUPPORTER, "",
							   "https://twitch.tv/thedangerbeard"}, // https://www.patreon.com/thedangerbeard
	streamfx::ui::about::entry{"GranDroidTonight", streamfx::ui::about::role_type::SUPPORTER, "",
							   "https://www.twitch.tv/GranDroidTonight"}, // https://github.com/GranDroidTonight
	streamfx::ui::about::entry{"Jahan", streamfx::ui::about::role_type::SUPPORTER, "",
							   "https://twitch.tv/1twohikaru"}, // https://www.patreon.com/user?u=152960
	streamfx::ui::about::entry{"Joefisx20s", streamfx::ui::about::role_type::SUPPORTER, "",
							   "https://www.twitch.tv/joefisx20s"}, // https://github.com/Joefis-x20s
	streamfx::ui::about::entry{"KrisCheetah", streamfx::ui::about::role_type::SUPPORTER, "",
							   ""}, // https://www.patreon.com/user?u=5208869
	streamfx::ui::about::entry{"ragesaq", streamfx::ui::about::role_type::SUPPORTER, "",
							   ""}, // https://www.patreon.com/user?u=34519727
	streamfx::ui::about::entry{"Rayxcer", streamfx::ui::about::role_type::SUPPORTER, "",
							   "https://twitter.com/Rayxcer1"}, // https://www.patreon.com/user?u=5086308
	streamfx::ui::about::entry{"SadeN", streamfx::ui::about::role_type::SUPPORTER, "",
							   "https://twitch.tv/saden_0"}, // https://www.patreon.com/vox_oculi
	streamfx::ui::about::entry{"SunsetsBrew", streamfx::ui::about::role_type::SUPPORTER, "",
							   "http://twitch.tv/torpidnetwork"}, // https://www.patreon.com/user?u=915823
	streamfx::ui::about::entry{"ThePooN", streamfx::ui::about::role_type::SUPPORTER, "",
							   "https://twitch.tv/ThePooN"}, // https://github.com/ThePooN
	streamfx::ui::about::entry{"tsukasa", streamfx::ui::about::role_type::SUPPORTER, "",
							   "https://tsukasa.eu/"}, // https://www.patreon.com/user?u=23383875
	streamfx::ui::about::entry{"Vensire Studios", streamfx::ui::about::role_type::SUPPORTER, "",
							   "https://www.facebook.com/vensirestudios"}, // https://github.com/VensireStudios
	streamfx::ui::about::entry{"y0himba", streamfx::ui::about::role_type::SUPPORTER, "",
							   "https://twitch.tv/y0himba"}, // https://www.patreon.com/y0himba

	// Separator
	streamfx::ui::about::entry{"", streamfx::ui::about::role_type::THANKYOU, "", ""},

	// Translators (Updated on 2021-04-23)
	streamfx::ui::about::entry{"Adolfo Jayme", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/fitoschido"}, // https://crowdin.com/profile/fitoschido
	streamfx::ui::about::entry{"Alex E. D. B.", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/alexedb"}, // https://crowdin.com/profile/alexedb
	streamfx::ui::about::entry{
		"Alexander Haffer", streamfx::ui::about::role_type::TRANSLATOR, "",
		"https://crowdin.com/profile/AlexanderHaffer"}, // https://crowdin.com/profile/AlexanderHaffer
	streamfx::ui::about::entry{"Arcan Yiğit Taşkan", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/karamizahim"}, // https://crowdin.com/profile/karamizahim
	streamfx::ui::about::entry{"Arda Gamer", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/noobumpls"}, // https://crowdin.com/profile/noobumpls
	streamfx::ui::about::entry{"Artem4ik", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/Artem4ik"}, // https://crowdin.com/profile/Artem4ik
	streamfx::ui::about::entry{"Arthur", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/Skor_X"}, // https://crowdin.com/profile/Skor_X
	streamfx::ui::about::entry{"Aurora Robb Kristiansen", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/Ridge"}, // https://crowdin.com/profile/Ridge
	streamfx::ui::about::entry{"Billy_la_menace", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/Symor"}, // https://crowdin.com/profile/Symor
	streamfx::ui::about::entry{
		"Boris Grigorov", streamfx::ui::about::role_type::TRANSLATOR, "",
		"https://crowdin.com/profile/boris.grigorov26"}, // https://crowdin.com/profile/boris.grigorov26
	streamfx::ui::about::entry{"Bruno Pinho", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/BPinho"}, // https://crowdin.com/profile/BPinho
	streamfx::ui::about::entry{"Cedric Günther", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/LeNinjaHD"}, // https://crowdin.com/profile/LeNinjaHD
	streamfx::ui::about::entry{"Chris BDS", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/HakuKaze"}, // https://crowdin.com/profile/HakuKaze
	streamfx::ui::about::entry{
		"Claudio Nunes", streamfx::ui::about::role_type::TRANSLATOR, "",
		"https://crowdin.com/profile/_claudionunes_"}, // https://crowdin.com/profile/_claudionunes_
	streamfx::ui::about::entry{"ColdusT", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/ColdusT"}, // https://crowdin.com/profile/ColdusT
	streamfx::ui::about::entry{"CyberspeedCz", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/CyberspeedCz"}, // https://crowdin.com/profile/CyberspeedCz
	streamfx::ui::about::entry{"Damian Soler", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/Under_RL"}, // https://crowdin.com/profile/Under_RL
	streamfx::ui::about::entry{"Danil Sarvensky", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/sarvensky"}, // https://crowdin.com/profile/sarvensky
	streamfx::ui::about::entry{"David Zheng", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/msdz"}, // https://crowdin.com/profile/msdz
	streamfx::ui::about::entry{"Deadbringer", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/Deadbringer"}, // https://crowdin.com/profile/Deadbringer
	streamfx::ui::about::entry{"DmJacky", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/DmJacky"}, // https://crowdin.com/profile/DmJacky
	streamfx::ui::about::entry{
		"EDNVKjldr8vyu9", streamfx::ui::about::role_type::TRANSLATOR, "",
		"https://crowdin.com/profile/EDNVKjldr8vyu9"}, // https://crowdin.com/profile/EDNVKjldr8vyu9
	streamfx::ui::about::entry{"EMRE ÇELİK", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/emre0447"}, // https://crowdin.com/profile/emre0447
	streamfx::ui::about::entry{"Emanuel Messias, R.d.S.", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/MessiasOF"}, // https://crowdin.com/profile/MessiasOF
	streamfx::ui::about::entry{"Enrique Castillo", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/ecastillo"}, // https://crowdin.com/profile/ecastillo
	streamfx::ui::about::entry{"Eryk Pazoła", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/erykpaz2004"}, // https://crowdin.com/profile/erykpaz2004
	streamfx::ui::about::entry{"Fire KM127PL", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/KM127PL"}, // https://crowdin.com/profile/KM127PL
	streamfx::ui::about::entry{"Francesco Maria Poerio", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/fracchiomon"}, // https://crowdin.com/profile/fracchiomon
	streamfx::ui::about::entry{"Frederico Maia", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/omaster"}, // https://crowdin.com/profile/omaster
	streamfx::ui::about::entry{
		"Gabriele Arena", streamfx::ui::about::role_type::TRANSLATOR, "",
		"https://crowdin.com/profile/teamexpert2013"}, // https://crowdin.com/profile/teamexpert2013
	streamfx::ui::about::entry{"Gol D. Ace", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/goldace"}, // https://crowdin.com/profile/goldace
	streamfx::ui::about::entry{"Guillaume Turchini", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/orion78fr"}, // https://crowdin.com/profile/orion78fr
	streamfx::ui::about::entry{"Hackebein", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/Hackebein"}, // https://crowdin.com/profile/Hackebein
	streamfx::ui::about::entry{"Holger Sinn", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/Holger-Sinn"}, // https://crowdin.com/profile/Holger-Sinn
	streamfx::ui::about::entry{"Itukii", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/Itukii"}, // https://crowdin.com/profile/Itukii
	streamfx::ui::about::entry{"Ivo Sestren Junior", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/IvoSestren"}, // https://crowdin.com/profile/IvoSestren
	streamfx::ui::about::entry{"Jeffrey Chapuis", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/ashyni1987"}, // https://crowdin.com/profile/ashyni1987
	streamfx::ui::about::entry{"Jelmer", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/Luxizzle"}, // https://crowdin.com/profile/Luxizzle
	streamfx::ui::about::entry{"Jens Fischer", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/JensFZ"}, // https://crowdin.com/profile/JensFZ
	streamfx::ui::about::entry{"Jessy HACHET", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/jessy.hachet"}, // https://crowdin.com/profile/jessy.hachet
	streamfx::ui::about::entry{"Job Diógenes Ribeiro Borges", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/jobdiogenes"}, // https://crowdin.com/profile/jobdiogenes
	streamfx::ui::about::entry{"Kim Bech", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/kimbech"}, // https://crowdin.com/profile/kimbech
	streamfx::ui::about::entry{"Kokosnuss.exe", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/KokosnussDEV"}, // https://crowdin.com/profile/KokosnussDEV
	streamfx::ui::about::entry{"Krit789", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/Krit789"}, // https://crowdin.com/profile/Krit789
	streamfx::ui::about::entry{"Kurozumi", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/Kurozumi"}, // https://crowdin.com/profile/Kurozumi
	streamfx::ui::about::entry{
		"LANCERRR", streamfx::ui::about::role_type::TRANSLATOR, "",
		"https://crowdin.com/profile/dongxujiayou88"}, // https://crowdin.com/profile/dongxujiayou88
	streamfx::ui::about::entry{"Leonardo Fries", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/LeoNerdinho"}, // https://crowdin.com/profile/LeoNerdinho
	streamfx::ui::about::entry{"Lifeely", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/Lifeely"}, // https://crowdin.com/profile/Lifeely
	streamfx::ui::about::entry{"Loïc", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/Ozachi"}, // https://crowdin.com/profile/Ozachi
	streamfx::ui::about::entry{"LucN31", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/LucN31"}, // https://crowdin.com/profile/LucN31
	streamfx::ui::about::entry{"Léo Roubaud", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/hydargos"}, // https://crowdin.com/profile/hydargos
	streamfx::ui::about::entry{"Maciej4535", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/Maciej4535"}, // https://crowdin.com/profile/Maciej4535
	streamfx::ui::about::entry{"Marco Túlio Pires", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/mtrpires"}, // https://crowdin.com/profile/mtrpires
	streamfx::ui::about::entry{"Martin Hybner", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/hybner"}, // https://crowdin.com/profile/hybner
	streamfx::ui::about::entry{"Michael Fabian Dirks", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/Xaymar"}, // https://crowdin.com/profile/Xaymar
	streamfx::ui::about::entry{"Mikhail", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/Migelius74"}, // https://crowdin.com/profile/Migelius74
	streamfx::ui::about::entry{"MkHere-YT", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/MkHere"}, // https://crowdin.com/profile/MkHere
	streamfx::ui::about::entry{"Mnr. Volkstaat", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/malapradex"}, // https://crowdin.com/profile/malapradex
	streamfx::ui::about::entry{"Monsteer", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/Monsteer"}, // https://crowdin.com/profile/Monsteer
	streamfx::ui::about::entry{"Mr Gohst", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/Gohst"}, // https://crowdin.com/profile/Gohst
	streamfx::ui::about::entry{"NOYB", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/NOYB"}, // https://crowdin.com/profile/NOYB
	streamfx::ui::about::entry{"Nanito Morillas", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/hellnano"}, // https://crowdin.com/profile/hellnano
	streamfx::ui::about::entry{"Nikola Perović", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/Fooftilly"}, // https://crowdin.com/profile/Fooftilly
	streamfx::ui::about::entry{"Nooody FR", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/nooodyfr"}, // https://crowdin.com/profile/nooodyfr
	streamfx::ui::about::entry{"Origami", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/Origami"}, // https://crowdin.com/profile/Origami
	streamfx::ui::about::entry{"Peter Grindem", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/1pete1"}, // https://crowdin.com/profile/1pete1
	streamfx::ui::about::entry{"Proryanator", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/Proryanator"}, // https://crowdin.com/profile/Proryanator
	streamfx::ui::about::entry{
		"Renaud G.", streamfx::ui::about::role_type::TRANSLATOR, "",
		"https://crowdin.com/profile/obiwankennedy"}, // https://crowdin.com/profile/obiwankennedy
	streamfx::ui::about::entry{
		"Richie Bendall", streamfx::ui::about::role_type::TRANSLATOR, "",
		"https://crowdin.com/profile/richiebendall"}, // https://crowdin.com/profile/richiebendall
	streamfx::ui::about::entry{"Romeo Bunić", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/Romz"}, // https://crowdin.com/profile/Romz
	streamfx::ui::about::entry{"SDUX1s44c", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/SDUX1s44c"}, // https://crowdin.com/profile/SDUX1s44c
	streamfx::ui::about::entry{"SKYQWER", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/mine17288"}, // https://crowdin.com/profile/mine17288
	streamfx::ui::about::entry{
		"SMG music display", streamfx::ui::about::role_type::TRANSLATOR, "",
		"https://crowdin.com/profile/SMGMusicDisplay"}, // https://crowdin.com/profile/SMGMusicDisplay
	streamfx::ui::about::entry{"Sade", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/Sade"}, // https://crowdin.com/profile/Sade
	streamfx::ui::about::entry{"Sandschi", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/sandschi"}, // https://crowdin.com/profile/sandschi
	streamfx::ui::about::entry{"ScottInTokyo", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/ScottInTokyo"}, // https://crowdin.com/profile/ScottInTokyo
	streamfx::ui::about::entry{"StanislavPro", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/StanislavPro"}, // https://crowdin.com/profile/StanislavPro
	streamfx::ui::about::entry{"StarFang208", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/StarFang208"}, // https://crowdin.com/profile/StarFang208
	streamfx::ui::about::entry{"Store", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/HelaBasa"}, // https://crowdin.com/profile/HelaBasa
	streamfx::ui::about::entry{"Syskoh", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/Syskoh"}, // https://crowdin.com/profile/Syskoh
	streamfx::ui::about::entry{
		"Szymon Szewc", streamfx::ui::about::role_type::TRANSLATOR, "",
		"https://crowdin.com/profile/szymonszewcjr"}, // https://crowdin.com/profile/szymonszewcjr
	streamfx::ui::about::entry{"TOWUK", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/towuk118"}, // https://crowdin.com/profile/towuk118
	streamfx::ui::about::entry{"ThePooN", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/ThePooN"}, // https://crowdin.com/profile/ThePooN
	streamfx::ui::about::entry{"Thom Knepper", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/MrKnepp"}, // https://crowdin.com/profile/MrKnepp
	streamfx::ui::about::entry{"Tim Cordes", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/t1mc0rd35"}, // https://crowdin.com/profile/t1mc0rd35
	streamfx::ui::about::entry{"TonyWin Somprasong", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/mga1627"}, // https://crowdin.com/profile/mga1627
	streamfx::ui::about::entry{"Tuna", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/zentoryny"}, // https://crowdin.com/profile/zentoryny
	streamfx::ui::about::entry{"Vane Brain", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/vanebrain3"}, // https://crowdin.com/profile/vanebrain3
	streamfx::ui::about::entry{"Viginox", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/Viginox"}, // https://crowdin.com/profile/Viginox
	streamfx::ui::about::entry{"WhiteyChannel", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/zvd.pro"}, // https://crowdin.com/profile/zvd.pro
	streamfx::ui::about::entry{"WoWnik", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/wownik98"}, // https://crowdin.com/profile/wownik98
	streamfx::ui::about::entry{"Yoinks DBD", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/izzyfazt"}, // https://crowdin.com/profile/izzyfazt
	streamfx::ui::about::entry{"Yurlyn", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/Yurlyn"}, // https://crowdin.com/profile/Yurlyn
	streamfx::ui::about::entry{"Zero Team", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/ZERO_TEAM"}, // https://crowdin.com/profile/ZERO_TEAM
	streamfx::ui::about::entry{"Zoop", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/Zoop"}, // https://crowdin.com/profile/Zoop
	streamfx::ui::about::entry{"erichardouin", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/erichardouin"}, // https://crowdin.com/profile/erichardouin
	streamfx::ui::about::entry{"exeldro", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/exeldro"}, // https://crowdin.com/profile/exeldro
	streamfx::ui::about::entry{"itsolutek", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/itsolutek"}, // https://crowdin.com/profile/itsolutek
	streamfx::ui::about::entry{"jh KIm", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/kkhmn1"}, // https://crowdin.com/profile/kkhmn1
	streamfx::ui::about::entry{
		"lukazivanovic", streamfx::ui::about::role_type::TRANSLATOR, "",
		"https://crowdin.com/profile/lukazivanovic"}, // https://crowdin.com/profile/lukazivanovic
	streamfx::ui::about::entry{"mochaaP", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/mochaaP"}, // https://crowdin.com/profile/mochaaP
	streamfx::ui::about::entry{"mudse", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/mudse"}, // https://crowdin.com/profile/mudse
	streamfx::ui::about::entry{"multi.flexi", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/multi.flexi"}, // https://crowdin.com/profile/multi.flexi
	streamfx::ui::about::entry{"mwessen", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/sh3ph3r6"}, // https://crowdin.com/profile/sh3ph3r6
	streamfx::ui::about::entry{"ozaki", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/OzakIOne"}, // https://crowdin.com/profile/OzakIOne
	streamfx::ui::about::entry{"petro770", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/petro770"}, // https://crowdin.com/profile/petro770
	streamfx::ui::about::entry{"rufus20145", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/rufus20145"}, // https://crowdin.com/profile/rufus20145
	streamfx::ui::about::entry{"sasagar", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/sasagar"}, // https://crowdin.com/profile/sasagar
	streamfx::ui::about::entry{"sasha2002", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/sasha2002"}, // https://crowdin.com/profile/sasha2002
	streamfx::ui::about::entry{"saygo1125", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/saygo1125"}, // https://crowdin.com/profile/saygo1125
	streamfx::ui::about::entry{"shugen002", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/shugen002"}, // https://crowdin.com/profile/shugen002
	streamfx::ui::about::entry{"spring jungle", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/springjungle"}, // https://crowdin.com/profile/springjungle
	streamfx::ui::about::entry{"tpo0508", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/tpo0508"}, // https://crowdin.com/profile/tpo0508
	streamfx::ui::about::entry{"tytan652", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/tytan652"}, // https://crowdin.com/profile/tytan652
	streamfx::ui::about::entry{
		"wwj402_github", streamfx::ui::about::role_type::TRANSLATOR, "",
		"https://crowdin.com/profile/wwj402_github"}, // https://crowdin.com/profile/wwj402_github
	streamfx::ui::about::entry{"Anonymous User", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/xaymarsdaddy"}, // https://crowdin.com/profile/xaymarsdaddy
	streamfx::ui::about::entry{"Вадим Казанцев", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/Rutboy"}, // https://crowdin.com/profile/Rutboy
	streamfx::ui::about::entry{"Вадим Якшин", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/tayler2013"}, // https://crowdin.com/profile/tayler2013
	streamfx::ui::about::entry{
		"Денис Полозихин", streamfx::ui::about::role_type::TRANSLATOR, "",
		"https://crowdin.com/profile/denispolozihin"}, // https://crowdin.com/profile/denispolozihin
	streamfx::ui::about::entry{
		"Дмитрий Балуев", streamfx::ui::about::role_type::TRANSLATOR, "",
		"https://crowdin.com/profile/darkinsonic13"}, // https://crowdin.com/profile/darkinsonic13
	streamfx::ui::about::entry{"Евгений Шестяков", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/anfogor"}, // https://crowdin.com/profile/anfogor
	streamfx::ui::about::entry{"十六夜楓", streamfx::ui::about::role_type::TRANSLATOR, "",
							   "https://crowdin.com/profile/fy1208557958"}, // https://crowdin.com/profile/fy1208557958

	// Separator
	streamfx::ui::about::entry{"", streamfx::ui::about::role_type::THANKYOU, "", ""},

	// Creators
	streamfx::ui::about::entry{"Andilippi", streamfx::ui::about::role_type::CREATOR, "",
							   "https://www.youtube.com/channel/UCp70l75kpG3ISyxpIsL6hfQ"},
	streamfx::ui::about::entry{"Axelle", streamfx::ui::about::role_type::CREATOR, "",
							   "https://www.twitch.tv/axelle123"},
	streamfx::ui::about::entry{"EposVox", streamfx::ui::about::role_type::CREATOR, "",
							   "https://www.youtube.com/channel/UCRBHiacaQb5S70pljtJYB2g"},
	streamfx::ui::about::entry{"Nordern", streamfx::ui::about::role_type::CREATOR, "",
							   "https://www.youtube.com/channel/UCX2I5pSP-b-iD8sPGiCO1dA"},
};

streamfx::ui::about::about() : QDialog(reinterpret_cast<QWidget*>(obs_frontend_get_main_window()))
{
	setupUi(this);

	// Remove some extra styling.
	setWindowFlag(Qt::WindowContextHelpButtonHint, false); // Remove the unimplemented help button.

	// Random thing
	auto            rnd = std::uniform_int_distribution(size_t(0), _thankyous.size() - 1);
	std::mt19937_64 rnde;

	// Thank every single helper.
	bool         column_selector = false;
	size_t       row_selector    = 0;
	QGridLayout* content_layout  = dynamic_cast<QGridLayout*>(content->layout());
	for (const auto& entry : _entries) {
		if (entry.role == role_type::SPACER) {
			row_selector += 2;
			column_selector = 0;

			// Add a separator line.
			auto separator = new QFrame(content);
			{
				auto sp = separator->sizePolicy();
				sp.setVerticalPolicy(QSizePolicy::Fixed);
				separator->setSizePolicy(sp);
			}
			separator->setFrameShape(QFrame::HLine);
			separator->setFrameShadow(QFrame::Sunken);
			separator->setMaximumHeight(1);
			separator->setMinimumHeight(1);
			separator->setFixedHeight(1);
			separator->setLineWidth(1);
			content_layout->addWidget(separator, static_cast<int>(row_selector - 1), 0, 1, 2);
			content_layout->setRowStretch(static_cast<int>(row_selector - 1), 0);
		} else if (entry.role == role_type::THANKYOU) {
			row_selector += 2;
			column_selector = 0;

			auto element = new QLabel(content);

			auto elrnd = rnd(rnde);
			element->setPixmap(QPixmap(_thankyous.at(elrnd).data()));

			element->setScaledContents(true);
			element->setFixedSize(384, 384);

			content_layout->addWidget(element, static_cast<int>(row_selector - 1), 0, 1, 2,
									  Qt::AlignTop | Qt::AlignHCenter);
			content_layout->setRowStretch(static_cast<int>(row_selector - 1), 0);
		} else {
			streamfx::ui::about_entry* v = new streamfx::ui::about_entry(content, entry);

			content_layout->addWidget(v, static_cast<int>(row_selector), column_selector ? 1 : 0);
			content_layout->setRowStretch(static_cast<int>(row_selector), 0);

			if (column_selector) {
				row_selector++;
			}
			column_selector = !column_selector;
		}
	}
	{
		row_selector++;
		auto padder = new QFrame(content);
		{
			auto sp = padder->sizePolicy();
			sp.setVerticalPolicy(QSizePolicy::Minimum);
			sp.setVerticalStretch(1);
			padder->setSizePolicy(sp);
		}
		padder->setObjectName("PaddleMeDaddy");
		padder->setMaximumHeight(QWIDGETSIZE_MAX);
		padder->setMinimumHeight(1);
		padder->setFrameShape(QFrame::NoFrame);
		content_layout->addWidget(padder, static_cast<int>(row_selector), 0, 1, 2);
		content_layout->setRowStretch(static_cast<int>(row_selector), 9999);
	}

	content_layout->setColumnStretch(0, 1);
	content_layout->setColumnStretch(1, 1);
	content->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Maximum);

	// Update the Version information.
	version->setText(STREAMFX_VERSION_STRING);

	// Make the OK button do things.
	connect(buttonBox, &QDialogButtonBox::accepted, this, &streamfx::ui::about::on_ok);
}

streamfx::ui::about::~about() {}

void streamfx::ui::about::on_ok()
{
	hide();
}
