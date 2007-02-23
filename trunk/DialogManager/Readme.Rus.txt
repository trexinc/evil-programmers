Плагин для FAR Manager Dialog Manager.

0. Лицензия.

DialogManager plugin for FAR Manager
Copyright (C) 2003 Vadim Yegorov
Copyright (C) 2004-2007 Vadim Yegorov and Alex Yaroslavsky

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

1. Введение.

Данный плагин служит хостом для своих субплагинов, позволяя расширять
стандартные возможности диалогов FAR Manager. После его установки в некоторых
диалогах появляется возможность по нажатию CtrlF11 получить меню со списком
подплагинов. Для удобства вызова этого меню прилагается файл F11.reg.

2. Версии.

2.1. Версия Windows.

NT4.0 и выше.

2.2. Версия FAR Manager.

1.70 build 1638 и выше.

3. Стандартные подплагины.

3.1 Case.

Позволяет изменять регистр текста в строках редактирования. Изменяется либо
регистр слова под курсором (или всего текста), либо выделения.

3.2. Pwd.

Позволяет поглядеть пароль за звездочками. Точно работает в FARMail, FarFTP,
FARNav и MultiArc.

3.3. OpenFile.

Показывает диалог выбора файла подобный стандартному диалогу OS. Полезен к
примеру при открытии файла в редакторе по ShiftF4, чтобы не терять текущие
директории на панелях.

3.4. Logger.

Пишет в лог все, что приходит в DefDlgProc всех диалогов всех фаров.
Пример:
Copy <0x00000868:0x000007b8> - 00126F48 0x1005 - [DN_DRAWDIALOG] 0x0 0x0
Copy - заголовок текущего окна фара.
<0x00000868:0x000007b8> - <ProcessID:ThreadID>.
00126F48 - hDlg.
0x1005 - Msg.
0x0 - Parm1.
0x0 - Parm2.

3.5. GrabDialog.

Сохраняет диалог в файл. Возможно два режима - простой и полный. Пример работы
в простом режиме:
InitDialogItem idi[] =
{
        /*Type,X1,Y1,X2,Y2,Param,Flags,Data*/
  /*00*/{DI_DOUBLEBOX,3,1,30,4,0,0,"Quit"},
  /*01*/{DI_TEXT,-1,2,0,2,0,DIF_SHOWAMPERSAND,"Do you want to quit FAR?"},
  /*02*/{DI_BUTTON,12,3,12,3,0,DIF_CENTERGROUP|DIF_NOBRACKETS," Yes "},
  /*03*/{DI_BUTTON,17,3,17,3,0,DIF_CENTERGROUP|DIF_NOBRACKETS," No "},
  /*DefaultButton=2, Focus=2*/
}

3.6. BCopy.

Показывает информационное меню плагина Background Copy. Для нормального
функционирования необходим билд плагина не ниже 51 и соответствующий параметр в
реестре:
REGEDIT4

[HKEY_CURRENT_USER\Software\Far\Plugins\BCopy]
"TechPreload"=dword:1

Также возможно потребуется очистка кэша плагинов.

3.7. CharMap.

Позволяет вставить в строку редактирования любой символ, используя плагин
Character Map. Для нормального функционирования необходима версия плагина не
ниже 3.1 и соответствующий параметр в реестре:
REGEDIT4

[HKEY_CURRENT_USER\Software\Far\Plugins\CharacterMap]
"Preload"=dword:1

Также возможно потребуется очистка кэша плагинов.

3.8. Macro.

Позволяет иметь для каждого диалога свой набор макросов. Существует также
возможность задать глобальные макросы для всех диалогов. Все макросы лежат в
ветке реестра:
HKEY_CURRENT_USER\Software\Far\Plugins\DialogM\Plugins\Macro
Внутри этого ключа может находиться любое количество подключей, каждый из
которых соответствует определенной группе диалогов.
Значение
[HKEY_CURRENT_USER\Software\Far\Plugins\DialogM\Plugins\Macro]
"Global"="@Global"
задает название глобальной группы.
Внутри каждой группы содержаться описания макросов. Например:
[HKEY_CURRENT_USER\Software\Far\Plugins\DialogM\Plugins\Macro\Findfile]
"Title"="Find file"
"Count"=dword:00000020
[HKEY_CURRENT_USER\Software\Far\Plugins\DialogM\Plugins\Macro\Findfile\F1@checkbox]
"Key"="F1"
"Sequence"="Space"
"Type"=dword:00000008
"DisableOutput"=dword:00000001
[HKEY_CURRENT_USER\Software\Far\Plugins\DialogM\Plugins\Macro\Findfile\F1@edit]
"Sequence"="A"
"Type"=dword:00000004
"Key"="F1"
"DisableOutput"=dword:00000000

Параметр Title определяет заголовок диалога, в котором будут срабатывать
макросы. Можно использовать маски, аналогичные фаровским для файлов.
Параметр Count определяет количество элементов в диалоге, в котором будут срабатывать
макросы.
Параметр Type определяет тип элемента диалога, для которого будет срабатывать
данный макрос.
Клавиша активации макроса может задаваться либо именем ключа, либо, в случае,
если одной и той же клавише присвоено несколько макросов с разными условиями,
из параметра Key.

Так как заголовок диалога не всегда может служить уникальным идентификатором
диалога, вы можете в своем плагине задать другой идентификатор в
обработчике сообщения DN_MACRO_GETDIALOGINFO следующим образом:
#include "dm_macro.hpp"
...
  switch(Msg)
  {
...
    case DN_MACRO_GETDIALOGINFO:
      MacroDialogInfo *mde=(MacroDialogInfo *)Param2;
      if(mde&&mde->StructSize>=(long)sizeof(MacroDialogInfo))
      {
        strcpy(mde->DialogId,"your_unique_value");
        return TRUE;
      }
      break;
...
  }

Так же можно запретить обработку макросов в каком либо диалоге следующим
образом:
#include "dm_macro.hpp"
...
  switch(Msg)
  {
...
    case DN_MACRO_DISABLE:
      return TRUE;
...
  }


3.9. Undo.

Реализует в строках ввода диалогов операции undo и redo. Количество откатов
неограниченно, но при потере строкой ввода фокуса они все теряются.

3.10. Paste Selection.

Позволяет в ставить в строку ввода текущее выделение в текущем редакторе.

3.11. Default Button.

Отмечает кнопку по умолчанию в диалоге взяв её в фигурные скобки.

4. Ограничения.

В некоторых диалогах комбинация CtrlF11 не срабатывает. Причиной является
функция-обработчик данного диалога, содержащая примерно следующий код:
long WINAPI DlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  ...
  switch(Msg)
  {
    case DN_KEY:
      if(Param2==KEY_???)
      {
        ...
        return TRUE;
      }
      else
        return FALSE;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}
Если его заменить на:
long WINAPI DlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  ...
  switch(Msg)
  {
    case DN_KEY:
      if(Param2==KEY_???)
      {
        ...
        return TRUE;
      }
      break;
  }
  return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}
то в работе диалога ничего не поменяется, а CtrlF11 начнет срабатывать.

5. Конфигурация.

Плагин знает о существовании некоторых ключей реестра:
REGEDIT4

[HKEY_CURRENT_USER\Software\Far\Plugins\DialogM]
; запретить плагину добавлять пункт в меню конфигурации плагинов.
"ShowInConfig"=dword:00000000
; запретить плагину добавлять пункт в меню плагинов.
"ShowInPanels"=dword:00000000
; кнопка для вызова плагина.
"HotKey"="CtrlAltF1"
; кнопка для выполнения int 3.
"DebugKey"="CtrlF12"

6. История.

v0.6 build 9 23-02-2007
- Добавлен субплагин: Replace
- Добавлен субплагин: Search
- Добавлен субплагин: Search and Replace
- charmap: добавлена работа в DI_FIXEDIT.

v0.6 build 8 23-02-2007
- grabber: обновление флагов DI_VTEXT.

v0.6 build 7 11-04-2005
- openfiledialog: исправления и оптимизации от Andrey Budko. Включает
  исправление падения плагина при попытке входа в блокированную папку.
- openfiledialog: после перехода в список дисков установим курсорную доску на
  активный до перехода диск.
- openfiledialog: мог не предвидено упасть при листании по папкам.
- macro: теперь понимает пустые заголовки диалогов.

v0.6 build 6 23-03-2005
- улучшена стабильность в поиске.
- английский перевод.
- case: опция для обработки всей строки, а не только слова под курсором.
- charmap: добавлена работа в DI_PSWEDIT.
- openfiledialog: исправлена работа с файлами имя которых длинней 128 символов.

v0.6 build 5 15-10-2004
- плагин не работал в билдах фара выше 1832.
- charmap: при изменении текста посылается DN_EDITCHANGE.
- case: при изменении текста посылается DN_EDITCHANGE.
- case: title case, cyclic change и работа над словом под курсором.
- pwd: пароль показывается в диалоге.
- pasteselection.
- undo.
- macro.
- defbutton.

v0.5 build 4 30-11-2003
- version info.
- charmap.
- logger: new messages.
- grabdialog: мелкие глюки в конфигурации.
- bcopy.
- grabdialog: неправильно открывался файл.
- openfiledialog: Left/Right работают как Home/End.
- openfiledialog: флаг DIF_LISTWRAPMODE убран.
- openfiledialog: диалог открытия файла подстраивает свой размер под
  размер консоли.
- openfiledialog: Shift-Enter по аналогии с фаром.
- openfiledialog: при нажатии на ".." курсор позиционируется на директории
  из которой вышли.

v0.0 build 3 23-04-2003
- настройки.
- grabdialog.
- logger.
- вызов из панелей.
- конфигурация.
- case: улучшена работа с блоками.
- минимальный требуемый билд фара - 1638.

v0.0 build 2 05-04-2003
- openfiledialog: вместо C:\Program Files\Far\Plugins\_Add\.. вставляется
  C:\Program Files\Far\Plugins\_Add\
- удаляется хоткей.
- если в языковом файле нет текущего языка, берем английский.

v0.0 build 1 06-03-2003
- первая версия.

7. Координаты.

Vadim Yegorov - at bmg.lv / zg
Alex Yaroslavsky - at yandex.ru / trexinc
