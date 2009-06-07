m4_include(`../FARMail/fm_version.m4')m4_dnl
.Language=Russian,Russian (Русский)
.PluginContents=Фильтр - плагин второго уровня для FARMail

@Contents
$^`#'Фильтр - плагин второго уровня для FARMail `v'MAJOR.MINOR#

 - ~Выбор по шаблону~@PatternFilter@
 - ~Конфигурация~@Config@
 - ~Авторы~@Authors@
 - ~Лицензия~@License@


@PatternFilter
$^#Выбор по шаблону#


  Есть возможность быстро выбрать сообщения, содержащие в заголовке
строку, попадающую под заданный шаблон. Нажав Alt-F6 в панели сообщений,
Вы увидите диалоговую панель:

 ╔═════════════════ Выбор сообщений ══════════════════╗
 ║                                                    ║
 ║ Искать выражение :                                 ║
 ║ from:*president@@mycountry.gov*                    ║
 ║                                                    ║
 ╟────────────────────────────────────────────────────╢
 ║                                                    ║
 ║ [ ] Искать в отмеченных сообщениях                 ║
 ║ [ ] Инверсия выборки                               ║
 ║ [ ] Учитывать регистр              [ Из файла... ] ║
 ║                                                    ║
 ╚════════════════════════════════════════════════════╝
                 [  Ок  ][  Отмена  ]


   Укажите шаблон для поиска в заголовках, используя общепринятые правила
(т.е., '?' означает строго один любой символ, '*' - 0 или больше любых
символов). Шаблон сравнивается последовательно с каждой строкой заголовка.
Можно искать только в выделенных сообщениях, искать сообщения, не подпадающие
под шаблон, а также искать с учетом регистра букв - нужно лишь отметить
соответствующий пункт. Данная возможность может быть полезна, если нужно
найти важное сообщение среди сотен лежащих на сервере, либо стереть
спам не читая. Например, чтобы быстро отыскать письмо с адреса
President@@MyCountry.gov, укажите шаблон:

 From:*president@@mycountry.gov*

  Звездочки в шаблоне очень полезны, так как реальная строка заголовка может
иметь вид:

 From:
  "John Doe" <president@@mycountry.gov> (а тут еще и комментарии)

  Если нужно стереть спам, и известно, что часть спама идет через сервер
с гордым именем "mail.suckingspammer.org", можно указать шаблон:

 Received:*from*mail.suckingspammer.org*by*

 и нажать F8, стерев все выбранные сообщения. Учтите, что я не несу
ответственности за потерю почты, вызванную неверным шаблоном или ошибкой
программы :) Так что может быть лучше будет поставить галочку у пункта
"Инверсия выборки", скопировать все отмеченные сообщения на диск, а спам
оставить для последующей обработки :)

  Если нет желания каждый раз при работе с почтой набирать вручную одно и
то же, можно создать фильтры в подкаталоге "Filters" и просто выбрать
"Из файла...". Появится менюшка со списком всех имеющихся фильтров. Фильтр -
это обычный текстовый файл с расширением "fmf", содержащий следующее:

   name     = <название фильтра, показываемое в меню>
   select   = <шаблон простановки выделения>
   unselect = <шаблон снятия выделения>

Строки, начинающиеся с ';' игнорируются.

Можно задать "горячую клавишу", проставив символ "&" перед нужной буквой
в названии фильтра. Сообщения, попадающие под шаблон в "select", выделяются,
а с тех, которые попали под шаблон "unselect", выделение снимается. Чем позже
объявлен шаблон, тем выше приоритет. Например, если нужно иметь возможность
выделять спам по Ctrl-S, нужно сделать следующее:

1. Создать фильтр, например spam.fmf в директории "filters", содержащий нечто
вроде:

  ; Название с "горячей клавишей"
  name = &Spam

  ; Сервера, с которых валится спам
  select=received:*from *.suckingspammer.org *by*
  select=received:*from *.damnspamsender.com *by*

  ; Почтовые клиенты, используемые для рассылки спама
  select=X-Mailer:*CoolSpamMailer v10.1*

  ; Сообщения от админов пропускаем всегда
  unselect=from:*admin@@*


2. Записать макрос на клавишу Ctrl-S, или запустить reg-файл, содержащий
что-то вроде:

  REGEDIT4

  [HKEY_CURRENT_USER\Software\Far\KeyMacros\Shell\CtrlS]
  "Sequence"="AltF6 AltU s"
  "DisableOutput"=dword:00000001

(AltU используется в предположении, что язык интерфейса - английский).

Вот собственно и все, теперь при нажатии Ctrl-S спам, попавший под
фильтр, будет выделен.



 ~Содержание~@Contents@


@Config
$^#Конфигурация#

#Папка фильтров# - здесь указывается полный путь к папке, в которой будут
искаться фильтры. Если это поле пустое, то фильтры ищутся в подпапке FILTERS
папки, в которой находится плагин.



 ~Содержание~@Contents@


@Authors
$^#Авторы#

  На сей момент разработкой плагина занимается FARMail Group:

  #Алексей Ярославский#
  Homepage : ~http://code.google.com/p/evil-programmers/~@http://code.google.com/p/evil-programmers/@

  #Вадим Егоров#
  E-mail   : at bmg.lv / zg
  Homepage : ~http://zg.times.lv/~@http://zg.times.lv/@

  В 1999-ом и 2000-ом годах разработкой плагина занимался его первоначальный
автор:

  #Сергей Александров#
  E-mail  : at zmail.ru / poseidon
  Homepage: ~http://alsea.euro.ru/~@http://alsea.euro.ru/@
            ~http://dpidb.genebee.msu.ru/users/poseidon/~@http://dpidb.genebee.msu.ru/users/poseidon/@



 ~Содержание~@Contents@

@License
$ #Лицензия#

 Фильтр - плагин второго уровня для FARMail
 Copyright (C) COPYRIGHT FARMail Group
 Copyright (C) 1999,2000 Serge Alexandrov

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

 ~Содержание~@Contents@
