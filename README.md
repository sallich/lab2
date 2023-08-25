# lab2 ИТМО ОСП 4 сем
## О проекте:
* сервер, поддерживающий многопоточность, транспортный протокол TCP и прикладной протокол (Запрос: 7 байтов, первые 6 байтов содержат некоторый MAC-адрес, последний
байт кодирует тип запроса (добавить, удалить, проверить).
Ответ, если ошибок не было: для операций добавления и удаления — 1 байт со
значением 0; для операции проверки — 1 байт со значением 0, если указанный
MAC-адрес не добавлен, и со значением 1 — если добавлен.
Ответ, если были ошибки: 1 байт со значением, отличным от 0 и 1, кодирующим
ошибку.
Дополнительные требования: добавленные MAC-адреса должны сохраняться
сервером между перезапусками.);
* клиент, поддерживающий заданный вариантом протокол и предназначенный для
тестирования сервера.