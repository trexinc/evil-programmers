m4_include(`../FARMail/fm_version.m4')m4_dnl
.Language=Russian,Russian (Русский)
.PluginContents= Адресная книга - плагин второго уровня для FARMail

@Contents
$^`#'Адресная книга - плагин второго уровня для FARMail `v'MAJOR.MINOR#

 - ~Как этим пользоваться~@AdrBook@
 - ~Параметры адресной книги~@AddrBookSet@
 - ~Авторы~@Authors@
 - ~Лицензия~@License@


@AdrBook
$^#Как этим пользоваться#


  Адресная книга - это всего лишь список получателей с именами, адресами и
комментариями. Нажмите "(?)" в диалоге отправки, чтобы увидеть полный список
получателей. Можно ввести новую запись по клавише F7, отредактировать
имеющуюся - F4, удалить (восстановить) запись - F8. Нажмите ENTER, чтобы
выбрать получателя письма. Можно выбрать нескольких получателей, отметив
записи INSERT-ом и нажав ENTER. Вы также можете сократить список введя
некоторый текст, список будет профильтрован и только те пункты которые
содержат введенный текст (где ни будь в имени, в электронном адресе или в
комментарии) будут показаны для быстрого доступа.


  Если в адресной книге нет записей, то при запуске Вам будет предложено
ввести новую запись.

  Редактировать адресную книгу можно в любой момент по нажатию Shift-F1 в
плагиновой панели или открыв плагин из редактора, если плагин был открыт из
редактора то при нажатии на ENTER выбранные записи будут вставлены в редактор
в текущей позиций.

  Название, адрес и комментарий должны быть менее 80 знаков.

  Возможна настройка полей адресной книги, поля сортировки и разделителя
записей.

  Адресная книга это всего лишь текстовый файл. Каждая строчка в нем -
отдельная запись. Можно редактировать этот файл в любом текстовом редакторе,
название, адрес и комментарий должны быть разделены символом "|". Сортировать
записи не нужно, это сделает программа. Порядок полей и разделитель можно
поменять в ~настройках программы~@AddrBookSet@. Строки, начинающиеся с ';',
игнорируются.



 ~Содержание~@Contents@


@AddrBookSet
$^#Параметры адресной книги#

 ╔═════ Параметры FARMail: Адресная книга ══════╗
 ║ Порядок полей:                           NEC ║
 ║ Разделитель полей:                         | ║
 ║ Сортировать по:                              ║
 ║  () имени                                   ║
 ║  ( ) E-Mail                                  ║
 ║  ( ) комментарию                             ║
 ║ Папка файла-адресная книга:                  ║
 ║ C:\Far\FARMail\FMP\AddressBook\              ║
 ╟──────────────────────────────────────────────╢
 ║              [ ОК ]  [ Отмена ]              ║
 ╚══════════════════════════════════════════════╝

#Порядок полей# - порядок следования полей в адресной книге (addressbook.adr):
                    N - имя
                    E - e-mail
                    C - комментарий

#Разделитель полей# - символ, которым разделяются поля записи в адресной книге.
По умолчанию "|".

#Сортировать по..# - поле, по которому сортируются записи при выводе на экран.

Замечание: при изменении порядка полей или разделителя адресная книга
не конвертируется!

#Папка файла-адресная книга# - Полный путь к папке где находится или будет
находится файл addressbook.adr (который и есть адресная книга). Пустое значение
этого поля соответствует директории плагина.



 ~Содержание~@Contents@


@Authors
$^#Авторы#

  На сей момент разработкой плагина занимается FARMail Group:

  #Алексей Ярославский#
  E-mail   : at yandex.ru / trexinc
  Homepage : ~http://trexinc.sf.net/~@http://trexinc.sf.net/@

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

 Адресная книга - плагин второго уровня для FARMail
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
