# Генерация графа состояний по исходникам С++



Для получения doxygen html Выполнить скрипты из `scripts` - `generate_puml.bat`, `run_doxygen.bat`. Иллюстрация:

```bash
mkdir bld
cd scripts
python fsm_comment_to_statechart.py ..\src
cd ..\src
doxygen.exe ..\Doxyfile
```

Внимание! Для запуска doxygen нужен установленный plantuml и dot, см. инструкцию [Doxygen Manual: Configuration](https://www.doxygen.nl/manual/config.html#cfg_plantuml_jar_path)


