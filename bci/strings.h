/**
	strings.h
	Copyright (C) 2009 GrAnD

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
**/
#ifndef STRINGS_HPP
#define STRINGS_HPP

PCWSTR			MsgMenu1033[] = {
	L"",
	L"About",
	L"Info",
	L"Cancel",
	L"Pause/Continue",
	L"Exit",
};
PCWSTR			MsgMenu1049[] = {
	L"",
	L"О программе",
	L"Информация",
	L"Отменить",
	L"Пауза/Продолжить",
	L"Выход",
};

PCWSTR			MsgOut1033[] = {
//	L"Invalid",
	L"Copying",
	L"Moving",
	L"Wiping",
	L"Deleting",
	L"Setting attributes",
};
PCWSTR			MsgOut1049[] = {
//	L"Invalid",
	L"Копирование",
	L"Перенос",
	L"Стирание",
	L"Удаление",
	L"Установка атрибутов",
};

PCWSTR			MsgAsk1033[] = {
	L"Attention",
	L"Destination already exists",
	L"The process cannot access the file",
	L"ASKGROUP_RETRYONLY",
	L"Symbolic link found",
};
PCWSTR			MsgAsk1049[] = {
	L"Внимание",
	L"Такой файл уже существует",
	L"Нет доступа к файлу",
	L"ASKGROUP_RETRYONLY",
	L"Найдена символическая связь",
};

PCWSTR			MsgInfo1033 = L"Info";
PCWSTR			MsgInfo1049 = L"Информация";

PCWSTR			MsgAboutTitle1033 = L"About";
PCWSTR			MsgAboutTitle1049 = L"О программе";
PCWSTR			MsgAbout1033 = L"bci - Background Copy Indicator\n\
	to use in conjunction with Background Copy service\n\
	(BCN.DLL must be placed near BCSVC.EXE)\n\
\n\
Purpose:\n\
	Displays BC operations' progressbar(s) in tray icons\n\
\n\
You can view operation' details in mouse hint\n\
If some operation needs user input (eg if destination already exists) then balloon pops out\n\
Extra options are available from icon' context menu (pause/resume, cancel, info)\n\
\n\
Usage:\n\
\n\
bci.exe [/nb] [/s \"wav-file\" [/t \"seconds\"] [/spf \"hertz\"] [/spd \"millisec\"]] [/w]\n\
bci.exe /r\n\
bci.exe /?\n\
\n\
/r		remove running bci.exe from memory\n\
/?		this help\n\
\n\
/e		enforce english dialogs\n\
/nb		do not display balloons\n\
/s \"wav-file\"	play sound after finishing operation\n\
		if file name is beep speaker will sound\n\
		sound plays only if operation lasts > timout\n\
/t \"seconds\"	set timeout 0..65535 (default 60)\n\
/spf \"hertz\"	speaker frequency (default 1000)\n\
/spd \"millisec\"	speaker duration (default 1000)\n\
/w		white tray icon (default black)";
PCWSTR			MsgAbout1049 = L"bci - Background Copy Indicator\n\
	приложение для использования совместно с сервисом Background Copy\n\
	(BCN.DLL должна находиться в той же директории что и BCSVC.EXE)\n\
\n\
Назначение:\n\
	Отображает очередь задач BC в трее\n\
\n\
Вы можете посмотреть детали задачи, просто наведите курсор мыши на нужную иконку\n\
Если какая либо из задач потребует вмешательства пользователя над ее иконкой появится всплывающее окно\n\
Управлять задачами можно с помощью щелчка правой кнопкой мыши (Пауза/Продолжить, Отменить, Инфо)\n\
\n\
Использование:\n\
\n\
bci.exe [/nb] [/s \"имя_файла\" [/t \"секунды\"] [/spf \"герц\"] [/spd \"милисек\"]] [/w]\n\
bci.exe /r\n\
bci.exe /?\n\
\n\
/r		Завершить процесс bci.exe, выполняющийся в данный момент\n\
/?		показать краткую справку\n\
\n\
/e		принудительно выводить все на английском языке\n\
/nb		отключить всплывающие окна\n\
/s \"имя_файла\"	Имя файла, для подачи сигнала при завершении операции\n\
		Если в качестве имени файла указать beep cигнал будет подаваться спикером\n\
		сигнал подается в случае если операция выполнялась дольше таймаут секунд\n\
/t \"секунды\"	установить таймаут в секундах 0..65535 (по умолчанию 60)\n\
/spf \"герц\"	частота спикера (по умолчанию 1000)\n\
/spd \"милисек\"	продолжительность сигнала спикера (по умолчанию 1000)\n\
/w		Рисовать иконку белым цветом (по умолчанию черным)";
PCWSTR			InfoTemplate1033 = L"%s\n%s\nfrom\n%s\nto\n%s";
PCWSTR			InfoTemplate1049 = L"%s\n%s\nиз\n%s\nв\n%s";

#endif // STRINGS_HPP
