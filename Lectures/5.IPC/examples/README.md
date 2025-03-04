# Тема 5. Межпроцессное взаимодействие

## Общие замечания

Все программы, за исключением 2_guard_page, 5_semaphore и 8_pipe_buffer, решают одну и ту же задачу.

Программа создает дочерний процесс, который является процессом-читателем.  
Родительский процесс становится процессом-писателем.

Процесс-писатель бесконечно считывает сообщение, вводимое пользователем,
и информирует процесс-читатель о наличии нового сообщения.

Процесс-читатель распечатывает сообщение и информирует процесс-писатель о том, что сообщение обработано.


## 1_file

В рамках данной программы для коммуникации между процессами используется обычный файл.
В качестве механизма синхронизации используется блокировка файла вызовом `flock()`.

Процесс-писатель считывает введенное сообщение, дожидается, когда файл станет пуст,
и записывает сообщение в файл.

Процесс-читатель дожидается, когда файл станет не пуст, считывает сообщение из файла,
и стирает содержимое файла.

## 3_mmap

В рамках данной программы для коммуникации между процессами используется отображение обычного файла
в память.
В качестве механизма синхронизации используются сигналы: 
`SIGUSR1` - сообщение записано, и `SIGUSR2` - сообщение обработано.

Процесс-писатель считывает введенное сообщение, записывает его в общую память, 
посылает `SIGUSR1` и дожидается `SIGUSR2`.

Процесс-читатель дожидается сигнала `SIGUSR1`, распечатывает сообщение и посылает `SIGUSR2`.

Т.к. отображение в память сохраняется при `fork()`, вызов `mmap()` производится до `fork()`.

## 4_shm

Программа является модификацией предыдущего примера с той лишь разницей, что вместо обычного файла 
в память отображается файл разделяемой памяти, который создаетв в начале программы вызовом `shm_open()`.

Обратите внимание, что имя файла разделяемой памяти должно начинаться с /.

## 6_pipe

В рамках программы для коммуникации между процессами используется неименованный канал.
Т.к. каналы обеспечивают и синхронизацию, и детектирование "обрыва" канала при уничтожении одного из процессов,
другие дополнительные средства синхронизации не используются.

Неименованный канал создается вызовом `pipe()` до `fork()`. При этом обратите внимание, что
на строках 42 и 46 процессы закрывают ненужные концы канала. Это необходимо, т.к. иначе не будет
работать детектирование "обрыва".

## 7_named_pipe

Программа является модификацией предыдущей программы с той лишь разницей, 
что вместо неименованного канала используется именованный канал.

Как следствие, отличается процесс создания канала (`mkfifo()` вместо `pipe()`) и получение дескрипторов.
Вместо того чтобы получить сразу 2 дескриптора, и потом закрыть ненужный,
каждый процесс самостоятельно открывает канал с нужными правами.

При этом программы синхронизируются автоматически, т.к. `open()` приостановит процесс до тех пор,
пока оба конца канала не будут открыты.

## 9_mqueue

В рамках программы для коммуникации между процессами используется очередь сообщений.

До вызова `fork()` очередь сообщений открывается и создается вызовом `mq_open()`.
Для очереди сообщений устанавливается максимальное количество ожидающих сообщений, равное 1, 
и максимальный размер сообщения, равный размеру структуры Message (строка 45).

По аналогии с объектами разделяемой памяти, имя очереди сообщений должно начинаться с /.

Т.к., чтение из пустой очереди и запись в полную очередь блокируют поток, дополнительных инструментов для
синхронизации читателя и писателя не требуется.

## 2_guard_page

Программа демонстрирует использование вызова `mmap()` 
для создания guard pages для защиты от переполнения.

Guard page - страница, любая попытка доступа к которой приводит к аппаратному исключению.
Для создания такой страницы в конце некоторой области можно выделить вызовом `mmap()` N+1
страниц с разрешением `PROT_NONE`, а затем для N страниц изменить разрешения на требуемые
вызовом `mprotect()`. Последняя страница диапазона останется с разрешениями `PROT_NONE`, 
и любая попытка доступа к ней приведет к получению сигнала `SIGSEGV`.

Программа выделяет 2 буфера, размером 16 байт, считывает в первый буфер пользовательский ввод,
выводит оба буфера и освобождает память.

Если у программы нет аргументов, то используются обычные функции выделения/удаления памяти.
При этом программа намеренно уязвима для переполнения буфера, 
чего можно добиться, введя строку длиной более 15.

Если у программы есть аргументы, то попытка переполнения приведет к попытке записи в guard page,
что приведет к аварийному завершению программы.

## 5_semaphore

Программа демонстрирует пример использования семафора для синхронизации доступа к общим данным.

В качестве общей переменной выступает целочисленная переменная из общей области памяти, 
которая одновременно инкрементируется в 2 процессах.

Программа создает семафор с исходным значением 1 вызовом `sem_open()` 
После этого программа создает файл разделяемой памяти и отображает его в память.

Далее программа создает дочерний процесс, после чего 2 процесса параллельно многократно инкрементируют переменную
без синхронизации. После этого дочерний процесс завершается, а родительский процесс выводит результат.

Далее программа вновь создает дочерний процесс, после чего 2 процесса параллельно многократно инкрементируют переменную
С синхронизацией посредством семафора (`sem_wait()`-инкремент-`sem_post()`). 
После этого дочерний процесс завершается, а родительский процесс выводит результат. 


Т.к. инкремент - это составная операция, без синхронизации результат работы получается неверным.

## 8_pipe_buffer

Программа демонстрирует эффектны неатомарной записи в канал.

Гарантируется, что запись в канал размером, менее `PIPE_BUF`, происходит атомарно. 
В противном случае записываемые данные могут быть разбиты на части и перемешаны. 
В результате возникает риск прочтения "битых" данных.

Значение `PIPE_BUF` зависит от системы, поэтому лучше завязываться на соответствующий макрос из `limits.h`.

В программе  N процессов-писателей записывают сообщения в канал, которые затем обрабатываются процессом-читателем.
Т.к. известно, что сообщения однородные, легко проверить целостность сообщения.

В случае сообщений размером менее `PIPE_BUF` нарушений целостности не наблюдается.
В случае сообщений размером более `PIPE_BUF` некоторые сообщения теряют целостность.

