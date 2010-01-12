m4_include(`../FARMail/fm_version.m4')m4_dnl
.Language=Russian,Russian (Русский)
.PluginContents=Помощник - плагин второго уровня для FARMail

@Contents
$^`#'Помощник - плагин второго уровня для FARMail `v'MAJOR.MINOR#

 - ~Как этим пользоваться~@Usage@
 - ~Диалог "открыть файл"~@FileDialog@
 - ~Конфигурация~@Config@
 - ~Авторы~@Authors@
 - ~Лицензия~@License@


@Usage
$^#Как этим пользоваться#

 #1. Прикрепить# - позволяет выбрать файл для аттача. Директива для аттача
вставляется в конец блока %start%-%end% или в текущую позицию курсора, если
такой блок отсутствует.

 #2. Вставить из файла# - позволяет выбрать файл, содержимое которого вставляется
в текущую позицию курсора. Содержимое файла обрамляется строками, определяемыми
в ~Конфигурации~@Config@.

 #3. Вставить из буфера обмена# - если буфер обмена не пуст, вставляет две строки,
определяемые в ~Конфигурации~@Config@ и позиционирует курсор для корректной вставки
содержания буфера обмена.

 #4. Версия FAR# - вставляет версию FAR в текущую позицию курсора.

 #5. Версия FARMail# - вставляет версию FARMail в текущую позицию курсора.

 #6. Версия Windows# - вставляет версию Windows в текущую позицию курсора.



 ~Содержание~@Contents@


@FileDialog
$^#Диалог "открыть файл"#

 Здесь Вы видите список файлов и директорий. Вы можете просматривать все ваши
диски, чтобы найти необходимый файл. Директории помечены символом "+".

 #Клавиши#

 #Enter# - Изменить директорию или выбрать файл.

 #Shift-Enter# - выполнить файл в новом окне.

 #Ctrl-\# - Показать список всех доступных дисков.

 #Alt-Буква# - Перейти в текущий каталог на диске "Буква".

 #правыйCtrl0-правыйCtrl9# - Как в ФАРе, перейти по ссылке на папку.

 #Shift-Enter# - Запуск в отдельном окне.



 ~Содержание~@Contents@


@Config
$^#Конфигурация#

 #Начало буфера обмена# и #Конец буфера обмена# - строки, используемые
командой ~Вставить из буфера обмена~@Usage@.

 #Начало файла# и #Конец файла# - строки, используемые командой
~Вставить из файла~@Usage@. Вместо первого символа "%s" вставляется полное имя
вставляемого файла.



 ~Содержание~@Contents@


@Authors
$^#Авторы#

  На сей момент разработкой плагина занимается FARMail Group:

  #Алексей Ярославский#
  Homepage : ~http://code.google.com/p/evil-programmers/~@http://code.google.com/p/evil-programmers/@

  #Вадим Егоров#
  E-mail   : at bmg.lv / zg
  Homepage : ~http://zg.times.lv/~@http://zg.times.lv/@



 ~Содержание~@Contents@

@License
$ #Лицензия#

 Помощник - плагин второго уровня для FARMail
 Copyright (C) COPYRIGHT FARMail Group

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
