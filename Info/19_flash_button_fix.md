# Исправления для кнопки Flash в VSCode

## Проблема
Кнопка Flash в status bar VSCode перестала работать после изменений в проекте.

## Причина
- `st-flash` не находился в PATH при запуске задачи из VSCode
- Кнопка была в status bar, но задача не выполнялась из-за отсутствия пути

## Исправления

### 1. Добавление кнопки Flash в status bar (.vscode/settings.json)
```json
"statusBar.tasks": [
    {
        "label": "Flash",
        "task": "Flash",
        "priority": 100
    }
]
```

### 2. Исправление PATH в Makefile
Изменена цель flash для корректного нахождения st-flash:
```makefile
flash: $(BUILD_DIR)/$(TARGET).bin
	export PATH=/usr/local/bin:$PATH && st-flash write $(BUILD_DIR)/$(TARGET).bin 0x08000000
```

Теперь кнопка Flash работает через status bar VSCode.
