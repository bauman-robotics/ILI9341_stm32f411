/**
 * @file keyboard_layout.h
 * @brief Конфигурация и отрисовка раскладки QWERTY клавиатуры
 *
 * Этот файл содержит все константы раскладки, логику позиционирования и функции отрисовки
 * для интерфейса QWERTY клавиатуры. Он обеспечивает чистое разделение задач раскладки
 * от основной логики приложения.
 */

#ifndef KEYBOARD_LAYOUT_H
#define KEYBOARD_LAYOUT_H

#include "ili9341.h"
#include "fonts.h"
#include "config.h"
#include <string.h>

// =============================================================================
// РАЗМЕРЫ И КОНСТАНТЫ ДИСПЛЕЯ
// =============================================================================

/** @brief Ширина дисплея в альбомной ориентации */
#define DISPLAY_WIDTH  320
/** @brief Высота дисплея в альбомной ориентации */
#define DISPLAY_HEIGHT 240

// =============================================================================
// КОНФИГУРАЦИЯ ПОЛЯ ВВОДА ТЕКСТА
// =============================================================================

/** @brief X координата поля ввода текста (левый отступ) */
#define INPUT_FIELD_X         5
/** @brief Y координата поля ввода текста (верхний отступ) */
#define INPUT_FIELD_Y         10
/** @brief Ширина поля ввода текста (почти полная ширина минус отступы) */
#define INPUT_FIELD_WIDTH     310
/** @brief Высота поля ввода текста (увеличена для двух строк) */
#define INPUT_FIELD_HEIGHT    60

/** @brief Расстояние между текстовым полем и рядом цифр */
#define TEXT_FIELD_TO_NUMBERS_SPACING  5

// =============================================================================
// РАЗМЕРЫ И ОТСТУПЫ КЛАВИАТУРЫ
// =============================================================================

/** @brief Ширина каждой клавиши клавиатуры в пикселях */
#define KEY_WIDTH        28
/** @brief Высота каждой клавиши клавиатуры в пикселях */
#define KEY_HEIGHT       28
/** @brief Расстояние между клавишами в пикселях */
#define KEY_SPACING      2
/** @brief Начальная X координата для раскладки клавиатуры */
#define KEYBOARD_START_X 5

// =============================================================================
// РАСЧЕТЫ ПОЗИЦИОНИРОВАНИЯ КЛАВИАТУРЫ
// =============================================================================

/**
 * @brief Расчет Y координаты начала ряда цифр
 * Расположен непосредственно под полем ввода текста с небольшим отступом
 */
#define NUMBERS_ROW_Y (INPUT_FIELD_Y + INPUT_FIELD_HEIGHT + TEXT_FIELD_TO_NUMBERS_SPACING)

/**
 * @brief Расчет Y координаты начала основной клавиатуры
 * Обеспечивает визуальное разделение между рядом цифр и QWERTY клавиатурой
 * Увеличьте последнее число для большего отступа от цифрового ряда
 */
#define KEYBOARD_START_Y (INPUT_FIELD_Y + INPUT_FIELD_HEIGHT + 40)  // Было 25, теперь 40

/** @brief Расстояние между рядами клавиатуры */
#define ROW_SPACING 5

// =============================================================================
// Y-КООРДИНАТЫ РЯДОВ QWERTY КЛАВИАТУРЫ
// =============================================================================

/** @brief Y координата для ряда QWERTY (qwertyuiop) */
#define QWERTY_ROW_Y     KEYBOARD_START_Y
/** @brief Y координата для ряда ASDF (asdfghjkl) */
#define ASDF_ROW_Y       (QWERTY_ROW_Y + KEY_HEIGHT + KEY_SPACING + ROW_SPACING)
/** @brief Y координата для ряда ZXCV (zxcvbnm) */
#define ZXCV_ROW_Y       (ASDF_ROW_Y + KEY_HEIGHT + KEY_SPACING + ROW_SPACING)
/** @brief Y координата для клавиши ПРОБЕЛ */
#define SPACE_BAR_Y      (ZXCV_ROW_Y + KEY_HEIGHT + KEY_SPACING + 10)

// =============================================================================
// X-СДВИГИ РЯДОВ QWERTY КЛАВИАТУРЫ
// =============================================================================

/** @brief X сдвиг для ряда QWERTY (стандартная позиция) */
#define QWERTY_ROW_OFFSET  0
/** @brief X сдвиг для ряда ASDF (половина ширины клавиши для шахматного расположения) */
#define ASDF_ROW_OFFSET    ((KEY_WIDTH + KEY_SPACING) / 2)
/** @brief X сдвиг для ряда ZXCV (полная ширина клавиши для шахматного расположения) */
#define ZXCV_ROW_OFFSET    (KEY_WIDTH + KEY_SPACING)

// =============================================================================
// КОНФИГУРАЦИЯ КЛАВИШИ ПРОБЕЛ
// =============================================================================

/** @brief X координата клавиши пробел (центрирована с небольшим левым отступом) */
#define SPACE_BAR_X       (KEYBOARD_START_X + 2 * (KEY_WIDTH + KEY_SPACING))
/** @brief Ширина клавиши пробел (охватывает 5 позиций клавиш минус расстояние) */
#define SPACE_BAR_WIDTH   (5 * (KEY_WIDTH + KEY_SPACING) - KEY_SPACING)

// =============================================================================
// НИЖНИЙ РЯД ФУНКЦИОНАЛЬНЫХ КНОПОК
// =============================================================================

/** @brief Ширина функциональных кнопок в нижнем ряду */
#define FUNC_KEY_WIDTH    45  // 
/** @brief Высота функциональных кнопок */
#define FUNC_KEY_HEIGHT   30
/** @brief Отступ между функциональными кнопками */
#define FUNC_KEY_SPACING  6

/** @brief Y координата нижнего ряда функциональных кнопок */
#define FUNC_ROW_Y        220  // Тестируем позицию выше

/** @brief Пиктограммы для функциональных кнопок */
#define FUNC_BUTTONS_COUNT 6
static const char* func_button_labels[FUNC_BUTTONS_COUNT] = {
    "Mnu",    // ≡ Меню (гамбургер меню)
    "Up ",   // ⇧ Переключение регистра (стрелка вверх)
    "Sps",   // □ Пробел (прямоугольник)
    "Lng",   // A↔ Переключение языка (A со стрелками)
    "Bck",   // × Отмена (крестик)
    "Ent"    //,   // ↲ Ввод (стрелка возврата)
    
};

// =============================================================================
// ВЫБОР ЦВЕТОВОЙ ПАЛИТРЫ
// =============================================================================

/**
 * @brief Инициализация цветовой палитры на основе конфигурации
 * @param[out] border_color Цвет для рамок клавиш
 * @param[out] key_color Цвет для фона клавиш
 * @param[out] text_color Цвет для текста клавиш
 */
static inline void init_color_palette(uint16_t *border_color, uint16_t *key_color, uint16_t *text_color) {
    switch (KEYBOARD_PALETTE) {
        case KEYBOARD_PALETTE_CLASSIC:
            *border_color = ILI9341_WHITE;
            *key_color = ILI9341_CYAN;  // Возвращаем нормальный голубой цвет
            *text_color = ILI9341_BLACK;
            break;
        case KEYBOARD_PALETTE_DARK:
            *border_color = ILI9341_GRAY;
            *key_color = ILI9341_NAVY;
            *text_color = ILI9341_WHITE;
            break;
        case KEYBOARD_PALETTE_MODERN:
            *border_color = ILI9341_BLACK;
            *key_color = ILI9341_MAGENTA;
            *text_color = ILI9341_WHITE;
            break;
        default:
            *border_color = ILI9341_WHITE;
            *key_color = ILI9341_CYAN;  // Возвращаем нормальный голубой цвет
            *text_color = ILI9341_BLACK;
    }
}

// =============================================================================
// ФУНКЦИИ ОТРИСОВКИ
// =============================================================================

/**
 * @brief Безопасная отрисовка прямоугольника с проверкой границ экрана
 * @param x X координата верхнего левого угла
 * @param y Y координата верхнего левого угла
 * @param width Ширина прямоугольника
 * @param height Высота прямоугольника
 * @param color Цвет заливки
 */
static inline void safe_fill_rectangle(int x, int y, int width, int height, uint16_t color) {
    // Проверка границ экрана
    if (x < 0) {
        width += x;  // Уменьшаем ширину
        x = 0;       // Сдвигаем к левому краю
    }
    if (y < 0) {
        height += y; // Уменьшаем высоту
        y = 0;       // Сдвигаем к верхнему краю
    }
    if (x + width > DISPLAY_WIDTH) {
        width = DISPLAY_WIDTH - x;  // Ограничиваем ширину
    }
    if (y + height > DISPLAY_HEIGHT) {
        height = DISPLAY_HEIGHT - y;  // Ограничиваем высоту
    }

    // Рисуем только если размеры положительные
    if (width > 0 && height > 0) {
        // Для очень больших прямоугольников ограничиваем размер для избежания переполнения
        const int MAX_RECT_SIZE = 5000; // Уменьшаем до 5000 пикселей за раз
        int total_pixels = width * height;

        if (total_pixels > MAX_RECT_SIZE) {
            // Рисуем по частям для больших прямоугольников
            int rows_per_chunk = MAX_RECT_SIZE / width;
            if (rows_per_chunk < 1) rows_per_chunk = 1;

            int current_y = y;
            int remaining_height = height;

            while (remaining_height > 0) {
                int chunk_height = (remaining_height > rows_per_chunk) ? rows_per_chunk : remaining_height;
                ILI9341_FillRectangle(x, current_y, width, chunk_height, color);
                current_y += chunk_height;
                remaining_height -= chunk_height;
            }
        } else {
            ILI9341_FillRectangle(x, y, width, height, color);
        }
    }
}

/**
 * @brief Отрисовка отдельной клавиши с заданной меткой и размерами
 * @param x X координата клавиши
 * @param y Y координата клавиши
 * @param label Текстовая метка для клавиши
 * @param width_mult Множитель ширины (1 для обычной, 2 для двойной ширины и т.д.)
 * @param border_color Цвет для рамки клавиши
 * @param key_color Цвет для фона клавиши
 * @param text_color Цвет для текста клавиши
 */
static inline void draw_key(int x, int y, const char* label, int width_mult,
                           uint16_t border_color, uint16_t key_color, uint16_t text_color) {
    // Расчет реальной ширины на основе множителя
    int actual_width = KEY_WIDTH * width_mult + (width_mult - 1) * KEY_SPACING;

    // Отрисовка рамки клавиши (2px контур) с проверкой границ
    safe_fill_rectangle(x - 1, y - 1, actual_width + 2, KEY_HEIGHT + 2, border_color);

    // Отрисовка фона клавиши с проверкой границ
    safe_fill_rectangle(x, y, actual_width, KEY_HEIGHT, key_color);

    // Выбор шрифта и расчет позиционирования текста
    if (KEYBOARD_FONT == KEYBOARD_FONT_SMALL) {
        // Font1 размер 1: 5x7 пикселей на символ, 6px интервал
        int text_x = x + (actual_width - (int)strlen(label) * 6) / 2;
        int text_y = y + 6;
        ILI9341_DrawString(text_x, text_y, label, text_color, key_color, 1, Font1);
    } else if (KEYBOARD_FONT == KEYBOARD_FONT_MEDIUM) {
        // Font1 размер 2: 10x14 пикселей на символ, 12px интервал
        int text_x = x + (actual_width - (int)strlen(label) * 12) / 2;
        int text_y = y + 4;
        ILI9341_DrawString(text_x, text_y, label, text_color, key_color, 2, Font1);
    }
}

// =============================================================================
// ФУНКЦИИ ОТРИСОВКИ РАСКЛАДКИ
// =============================================================================

/**
 * @brief Отрисовка поля ввода текста в верхней части экрана
 * Рисует белый прямоугольник с рамкой и настроенным текстом в две строки
 */
static inline void render_text_input_field(void) {
    // Отрисовка рамки вокруг поля ввода (3px общая рамка)
    ILI9341_FillRectangle(INPUT_FIELD_X - 2, INPUT_FIELD_Y - 2,
                         INPUT_FIELD_WIDTH + 4, INPUT_FIELD_HEIGHT + 4,
                         ILI9341_WHITE);
    ILI9341_FillRectangle(INPUT_FIELD_X - 1, INPUT_FIELD_Y - 1,
                         INPUT_FIELD_WIDTH + 2, INPUT_FIELD_HEIGHT + 2,
                         ILI9341_BLACK);
    ILI9341_FillRectangle(INPUT_FIELD_X, INPUT_FIELD_Y,
                         INPUT_FIELD_WIDTH, INPUT_FIELD_HEIGHT,
                         ILI9341_WHITE);

    // Отрисовка настроенного текста (две строки, 22px расстояние по вертикали)
    ILI9341_DrawString(INPUT_FIELD_X + 10, INPUT_FIELD_Y + 8,
                      DISPLAY_TEXT_LINE1, ILI9341_BLACK, ILI9341_WHITE, 2, Font1);
    ILI9341_DrawString(INPUT_FIELD_X + 10, INPUT_FIELD_Y + 30,
                      DISPLAY_TEXT_LINE2, ILI9341_BLACK, ILI9341_WHITE, 2, Font1);
}

/**
 * @brief Отрисовка функциональной клавиши с заданными размерами
 * @param x X координата клавиши
 * @param y Y координата клавиши
 * @param label Текстовая метка для клавиши
 * @param border_color Цвет для рамки клавиши
 * @param key_color Цвет для фона клавиши
 * @param text_color Цвет для текста клавиши
 */
static inline void draw_func_key(int x, int y, const char* label,
                                uint16_t border_color, uint16_t key_color, uint16_t text_color) {
    // Временно отключаем safe_fill_rectangle для тестирования
    ILI9341_FillRectangle(x - 1, y - 1, FUNC_KEY_WIDTH + 2, FUNC_KEY_HEIGHT + 2, border_color);
    ILI9341_FillRectangle(x, y, FUNC_KEY_WIDTH, FUNC_KEY_HEIGHT, key_color);

    // Расчет позиционирования текста для функциональных клавиш
    int text_x = x + (FUNC_KEY_WIDTH - (int)strlen(label) * 12) / 2;  // Font1 size 2
    int text_y = y + 4;
    ILI9341_DrawString(text_x, text_y, label, text_color, key_color, 2, Font1);
}

/**
 * @brief Отрисовка нижнего ряда функциональных кнопок
 * Включает CAPS, SPACE, LANG, CANCEL, ENTER, MENU
 */
static inline void render_functional_buttons(void) {
    // Инициализация цветовой палитры
    uint16_t border_color, key_color, text_color;
    init_color_palette(&border_color, &key_color, &text_color);

    // Текущая X позиция для отрисовки функциональных кнопок
    int current_x = KEYBOARD_START_X;

    // Отрисовка 6 функциональных кнопок с текстом
    for (int i = 0; i < FUNC_BUTTONS_COUNT; i++) {
        draw_func_key(current_x, FUNC_ROW_Y, func_button_labels[i],
                     border_color, key_color, text_color);
        current_x += FUNC_KEY_WIDTH + FUNC_KEY_SPACING;
    }
}

/**
 * @brief Отрисовка полной раскладки QWERTY клавиатуры
 * Включает ряд цифр, раскладку QWERTY, пунктуацию и нижний ряд функциональных кнопок
 */
static inline void render_keyboard_layout(void) {
    // Инициализация цветовой палитры
    uint16_t border_color, key_color, text_color;
    init_color_palette(&border_color, &key_color, &text_color);

    // Текущая X позиция для отрисовки клавиш
    int current_x;

    // Отрисовка ряда цифр (1 2 3 4 5 6 7 8 9 0)
    const char *numbers = "1234567890";
    current_x = KEYBOARD_START_X;
    for (int i = 0; numbers[i] != '\0'; i++) {
        char key_label[2] = {numbers[i], '\0'};
        draw_key(current_x, NUMBERS_ROW_Y, key_label, 1, border_color, key_color, text_color);
        current_x += KEY_WIDTH + KEY_SPACING;
    }

    // Отрисовка ряда QWERTY (q w e r t y u i o p) или (Q W E R T Y U I O P)
    const char *qwerty1 = (KEYBOARD_CASE == KEYBOARD_CASE_LOWER) ? "qwertyuiop" : "QWERTYUIOP";
    current_x = KEYBOARD_START_X + QWERTY_ROW_OFFSET;
    for (int i = 0; qwerty1[i] != '\0'; i++) {
        char key_label[2] = {qwerty1[i], '\0'};
        draw_key(current_x, QWERTY_ROW_Y, key_label, 1, border_color, key_color, text_color);
        current_x += KEY_WIDTH + KEY_SPACING;
    }

    // Отрисовка ряда ASDF (a s d f g h j k l) или (A S D F G H J K L)
    const char *qwerty2 = (KEYBOARD_CASE == KEYBOARD_CASE_LOWER) ? "asdfghjkl" : "ASDFGHJKL";
    current_x = KEYBOARD_START_X + ASDF_ROW_OFFSET;
    for (int i = 0; qwerty2[i] != '\0'; i++) {
        char key_label[2] = {qwerty2[i], '\0'};
        draw_key(current_x, ASDF_ROW_Y, key_label, 1, border_color, key_color, text_color);
        current_x += KEY_WIDTH + KEY_SPACING;
    }

    // Отрисовка ряда ZXCV (z x c v b n m . ,) или (Z X C V B N M . ,)
    const char *qwerty3 = (KEYBOARD_CASE == KEYBOARD_CASE_LOWER) ? "zxcvbnm.," : "ZXCVBNM.,";
    current_x = KEYBOARD_START_X + ZXCV_ROW_OFFSET;
    for (int i = 0; qwerty3[i] != '\0'; i++) {
        char key_label[2] = {qwerty3[i], '\0'};
        draw_key(current_x, ZXCV_ROW_Y, key_label, 1, border_color, key_color, text_color);
        current_x += KEY_WIDTH + KEY_SPACING;
    }

    // Отрисовка нижнего ряда функциональных кнопок ПОСЛЕДНИМИ (поверх клавиатуры)
    render_functional_buttons();
}

// =============================================================================
// ГЛАВНАЯ ФУНКЦИЯ ОТРИСОВКИ
// =============================================================================

/**
 * @brief Детальное логирование координат всех элементов интерфейса
 * Выводит полную информацию о позиционировании для отладки
 */
static inline void log_framebuffer_coordinates(void) {
    LOG_SendString("=== FRAMEBUFFER COORDINATES DEBUG ===\r\n");

    // Общие параметры дисплея
    LOG_Printf("DISPLAY: Width=%d, Height=%d\r\n", DISPLAY_WIDTH, DISPLAY_HEIGHT);

    // Поле ввода текста
    LOG_SendString("TEXT INPUT FIELD:\r\n");
    LOG_Printf("  X=%d, Y=%d, Width=%d, Height=%d\r\n",
               INPUT_FIELD_X, INPUT_FIELD_Y, INPUT_FIELD_WIDTH, INPUT_FIELD_HEIGHT);
    LOG_Printf("  End X=%d, End Y=%d\r\n",
               INPUT_FIELD_X + INPUT_FIELD_WIDTH, INPUT_FIELD_Y + INPUT_FIELD_HEIGHT);

    // Цифровой ряд
    LOG_SendString("NUMBERS ROW:\r\n");
    LOG_Printf("  Y=%d, Height=%d\r\n", NUMBERS_ROW_Y, KEY_HEIGHT);
    LOG_Printf("  Keys: 10 buttons, Width=%d each, Spacing=%d\r\n", KEY_WIDTH, KEY_SPACING);
    LOG_Printf("  Total width: %d pixels\r\n", 10 * KEY_WIDTH + 9 * KEY_SPACING);

    // QWERTY ряд
    LOG_SendString("QWERTY ROW:\r\n");
    LOG_Printf("  Y=%d, Height=%d\r\n", QWERTY_ROW_Y, KEY_HEIGHT);
    LOG_Printf("  Keys: 10 buttons (q-p), Width=%d each\r\n", KEY_WIDTH);

    // ASDF ряд
    LOG_SendString("ASDF ROW:\r\n");
    LOG_Printf("  Y=%d, Height=%d\r\n", ASDF_ROW_Y, KEY_HEIGHT);
    LOG_Printf("  Keys: 9 buttons (a-l), Width=%d each\r\n", KEY_WIDTH);

    // ZXCV ряд
    LOG_SendString("ZXCV ROW:\r\n");
    LOG_Printf("  Y=%d, Height=%d\r\n", ZXCV_ROW_Y, KEY_HEIGHT);
    LOG_Printf("  Keys: 9 buttons (z-.,), Width=%d each\r\n", KEY_WIDTH);

    // Функциональные кнопки
    LOG_SendString("FUNCTIONAL BUTTONS:\r\n");
    LOG_Printf("  Y=%d, Height=%d\r\n", FUNC_ROW_Y, FUNC_KEY_HEIGHT);
    LOG_Printf("  Count=%d, Width=%d each, Spacing=%d\r\n",
               FUNC_BUTTONS_COUNT, FUNC_KEY_WIDTH, FUNC_KEY_SPACING);
    LOG_Printf("  Total width: %d pixels\r\n",
               FUNC_BUTTONS_COUNT * FUNC_KEY_WIDTH + (FUNC_BUTTONS_COUNT - 1) * FUNC_KEY_SPACING);

    // Схематичный вывод
    LOG_SendString("SCHEMATIC LAYOUT:\r\n");
    LOG_SendString("┌─────────────────────────────────────┐\r\n");
    LOG_SendString("│ ┌─────────────────────────────────┐ │\r\n");
    LOG_Printf("│ │        TEXT INPUT FIELD         │ │ Y=%d-%d\r\n", INPUT_FIELD_Y, INPUT_FIELD_Y + INPUT_FIELD_HEIGHT);
    LOG_SendString("│ └─────────────────────────────────┘ │\r\n");
    LOG_SendString("└─────────────────────────────────────┘\r\n");
    LOG_Printf("     ↓ %dpx\r\n", TEXT_FIELD_TO_NUMBERS_SPACING);
    LOG_Printf("┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐ Y=%d\r\n", NUMBERS_ROW_Y);
    LOG_SendString("│ 1 │ 2 │ 3 │ 4 │ 5 │ 6 │ 7 │ 8 │ 9 │ 0 │\r\n");
    LOG_SendString("└───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘\r\n");
    LOG_Printf("     ↓ %dpx\r\n", ROW_SPACING);
    LOG_Printf("┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐ Y=%d\r\n", QWERTY_ROW_Y);
    LOG_SendString("│ q │ w │ e │ r │ t │ y │ u │ i │ o │ p │\r\n");
    LOG_SendString("└───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘\r\n");
    LOG_Printf("     ↓ %dpx\r\n", ROW_SPACING);
    LOG_Printf("┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐ Y=%d\r\n", ASDF_ROW_Y);
    LOG_SendString("│ a │ s │ d │ f │ g │ h │ j │ k │ l │   │\r\n");
    LOG_SendString("└─────────────────────────────────────┘\r\n");
    LOG_Printf("     ↓ %dpx\r\n", ROW_SPACING);
    LOG_Printf("┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐ Y=%d\r\n", ZXCV_ROW_Y);
    LOG_SendString("│ z │ x │ c │ v │ b │ n │ m │ . │ , │     │\r\n");
    LOG_SendString("└─────────────────────────────────────────────┘\r\n");
    int gap_to_func = FUNC_ROW_Y - (ZXCV_ROW_Y + KEY_HEIGHT);
    LOG_Printf("     ↓ %dpx\r\n", gap_to_func);
    LOG_Printf("┌─────┬─────┬─────┬─────┬─────┬─────┐ Y=%d\r\n", FUNC_ROW_Y);
    LOG_SendString("│  ⇧  │  □  │ A↔  │  ×  │  ↲  │  ≡  │\r\n");
    LOG_SendString("└─────┴─────┴─────┴─────┴─────┴─────┘\r\n");
    LOG_SendString("=== END FRAMEBUFFER DEBUG ===\r\n");
}

/**
 * @brief Отрисовка полного интерфейса клавиатуры
 * Это основная функция для вызова из приложения
 */
static inline void render_keyboard_interface(void) {
    // Отладочное логирование координат если включено
    #if ENABLE_FRAMEBUFFER_DEBUG
    log_framebuffer_coordinates();
    #endif

    render_text_input_field();
    render_keyboard_layout();
}

#endif /* KEYBOARD_LAYOUT_H */
