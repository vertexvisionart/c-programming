#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Кроссплатформенная магия для загрузки динамических библиотек ---
#ifdef _WIN32
    #include <windows.h>
    #define LIB_EXT ".dll"
    typedef HMODULE LibHandle;
#else
    #include <dlfcn.h>
    #define LIB_EXT ".so"
    typedef void* LibHandle;
#endif

typedef struct State {
    char *regs[256];
} State;

State global_state = {0}; // Инициализируем всё нулями

// Кэш для загруженных плагинов
typedef struct {
    char name[64];
    LibHandle handle;
} PluginCache;

PluginCache plugins[64];
int plugin_count = 0;

// Функция загрузки библиотеки с учетом ОС
LibHandle load_library_os(const char* name) {
    char filename[256];
    sprintf(filename, "%s%s", name, LIB_EXT);
#ifdef _WIN32
    return LoadLibraryA(filename);
#else
    return dlopen(filename, RTLD_LAZY);
#endif
}

// Поиск плагина в кэше или его загрузка
LibHandle get_plugin(const char* name) {
    for (int i = 0; i < plugin_count; i++) {
        if (strcmp(plugins[i].name, name) == 0) {
            return plugins[i].handle;
        }
    }
    
    LibHandle h = load_library_os(name);
    if (h) {
        strcpy(plugins[plugin_count].name, name);
        plugins[plugin_count].handle = h;
        plugin_count++;
    }
    return h;
}

// Получение указателя на функцию
void* get_function(LibHandle h, const char* func_name) {
#ifdef _WIN32
    return (void*)GetProcAddress(h, func_name);
#else
    return dlsym(h, func_name);
#endif
}

// Типы функций для приведения указателей
typedef void (*Func0)(State*);
typedef void (*Func1)(State*, char*);
typedef void (*Func2)(State*, char*, char*);
typedef void (*Func3)(State*, char*, char*, char*);

int main() {
    char line[1024];
    
    while (fgets(line, sizeof(line), stdin)) {
        // Убираем символы переноса строки
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) == 0) continue;

        // Разбиваем строку на слова
        char *words[5];
        int wc = 0;
        char *token = strtok(line, " ");
        while (token && wc < 5) {
            words[wc++] = token;
            token = strtok(NULL, " ");
        }
        if (wc == 0) continue;

        // Парсим имя плагина и функции (words[0])
        char plugin_name[64] = "core"; // Значение по умолчанию
        char func_base[64];
        
        char *colon_pos = strchr(words[0], ':');
        if (colon_pos) {
            *colon_pos = '\0';
            strcpy(plugin_name, words[0]);
            strcpy(func_base, colon_pos + 1);
        } else {
            strcpy(func_base, words[0]);
        }

        int argc = wc - 1; // Количество аргументов (от 0 до 3)
        char func_full[128];
        sprintf(func_full, "%s_%d", func_base, argc);

        // 1. Пытаемся получить плагин
        LibHandle lib = get_plugin(plugin_name);
        if (!lib) {
            printf("Missing plugin %s\n", plugin_name);
            continue;
        }

        // 2. Пытаемся найти функцию
        void* func_ptr = get_function(lib, func_full);
        if (!func_ptr) {
            printf("Missing function %s in plugin %s\n", func_full, plugin_name);
            continue;
        }

        // 3. Выполняем функцию
        if (argc == 0) {
            ((Func0)func_ptr)(&global_state);
        } else if (argc == 1) {
            ((Func1)func_ptr)(&global_state, words[1]);
        } else if (argc == 2) {
            ((Func2)func_ptr)(&global_state, words[1], words[2]);
        } else if (argc == 3) {
            ((Func3)func_ptr)(&global_state, words[1], words[2], words[3]);
        }
    }

    return 0;
}