#!/usr/bin/env python3
"""
Применяет изменения поверх чистого кода AyuGram перед сборкой.

1. Иконки: копирует custom/icons/<имя>/ в Telegram/Resources/art/ayu/<имя>/
   и прописывает их в ayu.qrc, ayu_logo.h и icon_picker.cpp.
   Новая иконка = новая папка в custom/icons (app_icon.ico + app.png или app.svg).
2. Видимые названия: AyuGram -> Telegram (exe, свойства exe, имя программы,
   ярлыки, заголовки настроек, тексты интерфейса и загружаемые переводы).

Если какое-то место в коде не найдено, скрипт перечисляет все такие места
и завершается с ошибкой, чтобы сборка не прошла с неполными изменениями.

Запуск: python custom/apply.py  (из корня репозитория или откуда угодно)
        python custom/apply.py prepare  — только обходы для сборки библиотек
"""

import re
import shutil
import sys
from pathlib import Path

try:
    sys.stdout.reconfigure(encoding="utf-8")
except Exception:
    pass

ROOT = Path(__file__).resolve().parent.parent
ICONS_SRC = ROOT / "custom" / "icons"
ART_AYU = ROOT / "Telegram" / "Resources" / "art" / "ayu"

errors = []


def read(rel):
    with open(ROOT / rel, encoding="utf-8", newline="") as f:
        return f.read()


def write(rel, text):
    with open(ROOT / rel, "w", encoding="utf-8", newline="") as f:
        f.write(text)


def eol(text):
    return "\r\n" if "\r\n" in text else "\n"


def replace(rel, old, new, count=1):
    """Заменить ровно count вхождений old на new в файле rel."""
    text = read(rel)
    found = text.count(old)
    if found != count:
        errors.append(f"{rel}: ожидалось {count} вхожд. {old!r}, найдено {found}")
        return
    write(rel, text.replace(old, new))
    print(f"  {rel}: {old!r} -> {new!r}" + (f" (x{count})" if count > 1 else ""))


def insert_before(rel, anchor, line):
    """Вставить строку line перед единственной строкой, содержащей anchor."""
    text = read(rel)
    nl = eol(text)
    if text.count(anchor) != 1:
        errors.append(f"{rel}: якорь {anchor!r} найден {text.count(anchor)} раз(а), нужен 1")
        return
    idx = text.index(anchor)
    start = text.rfind("\n", 0, idx) + 1
    write(rel, text[:start] + line + nl + text[start:])
    print(f"  {rel}: вставлено {line.strip()!r}")


# ---------------------------------------------------------------- иконки

def apply_icons():
    print("Иконки:")
    names = sorted(p.name for p in ICONS_SRC.iterdir() if p.is_dir())
    if not names:
        errors.append("custom/icons: нет ни одной папки с иконкой")
        return

    entries = []  # (имя, [файлы])
    for name in names:
        src = ICONS_SRC / name
        if not re.fullmatch(r"[a-z][a-z0-9_]*", name):
            errors.append(f"custom/icons/{name}: имя должно быть из a-z, 0-9, _ и начинаться с буквы")
            continue
        files = [f for f in ("app.png", "app.svg", "app_icon.ico") if (src / f).exists()]
        if "app_icon.ico" not in files or not ({"app.png", "app.svg"} & set(files)):
            errors.append(f"custom/icons/{name}: нужны app_icon.ico и app.png (или app.svg)")
            continue
        if (ART_AYU / name).exists():
            errors.append(f"art/ayu/{name}: такая иконка уже есть в AyuGram, переименуй папку")
            continue
        entries.append((name, files))
    if errors:
        return

    for name, files in entries:
        shutil.copytree(ICONS_SRC / name, ART_AYU / name)
    print(f"  скопировано папок: {len(entries)}")

    # ayu.qrc: строки после последней art/ayu/...
    qrc = "Telegram/Resources/qrc/ayu/ayu.qrc"
    text = read(qrc)
    nl = eol(text)
    lines = text.split(nl)
    last = max((i for i, l in enumerate(lines) if 'alias="art/ayu/' in l), default=None)
    if last is None:
        errors.append(f"{qrc}: не найдены строки art/ayu/")
    else:
        indent = re.match(r"\s*", lines[last]).group(0)
        new = [
            f'{indent}<file alias="art/ayu/{n}/{f}">../../art/ayu/{n}/{f}</file>'
            for n, files in entries for f in files
        ]
        write(qrc, nl.join(lines[: last + 1] + new + lines[last + 1:]))
        print(f"  {qrc}: +{len(new)} строк")

    # ayu_logo.h: ICON(...) после последнего ICON(...)
    logo = "Telegram/SourceFiles/ayu/ui/ayu_logo.h"
    text = read(logo)
    nl = eol(text)
    lines = text.split(nl)
    last = max((i for i, l in enumerate(lines) if l.startswith("ICON(")), default=None)
    if last is None:
        errors.append(f"{logo}: не найдены строки ICON(...)")
    else:
        new = [f'ICON({n.upper()}, "{n}");' for n, _ in entries]
        write(logo, nl.join(lines[: last + 1] + new + lines[last + 1:]))
        print(f"  {logo}: +{len(new)} строк")

    # icon_picker.cpp: в конец списка icons{ ... };
    picker = "Telegram/SourceFiles/ayu/ui/components/icon_picker.cpp"
    text = read(picker)
    nl = eol(text)
    m = re.search(r"const QVector<QString> icons\{.*?(\r?\n)\};", text, re.S)
    if not m:
        errors.append(f"{picker}: не найден список 'const QVector<QString> icons{{ ... }};'")
    else:
        pos = m.end() - 2  # перед '};'
        new = "".join(f"\tAyuAssets::{n.upper()}_ICON,{nl}" for n, _ in entries)
        write(picker, text[:pos] + new + text[pos:])
        print(f"  {picker}: +{len(entries)} иконок в списке")


# ---------------------------------------------------- видимые названия

def apply_names():
    print("Названия:")
    # exe
    replace("Telegram/CMakeLists.txt", 'set(output_name "AyuGram")', 'set(output_name "Telegram")')
    replace("Telegram/CMakeLists.txt", 'STREQUAL "AyuGram")', 'STREQUAL "Telegram")')

    # свойства exe
    rc = "Telegram/Resources/winrc/Telegram.rc"
    replace(rc, '"CompanyName", "Radolyn Labs"', '"CompanyName", "Telegram FZ-LLC"')
    replace(rc, '"FileDescription", "AyuGram Desktop"', '"FileDescription", "Telegram Desktop"')
    replace(rc, '"ProductName", "AyuGram Desktop"', '"ProductName", "Telegram Desktop"')

    # имя программы (заголовки, папка данных, ярлык автозапуска)
    ver = "Telegram/SourceFiles/core/version.h"
    replace(ver, 'AppName = "AyuGram Desktop"_cs', 'AppName = "Telegram Desktop"_cs')
    replace(ver, 'AppFile = "AyuGram"_cs', 'AppFile = "Telegram"_cs')

    # ярлыки (меню Пуск, рабочий стол; по ним Windows подписывает уведомления)
    aumid = "Telegram/SourceFiles/platform/win/windows_app_user_model_id.cpp"
    replace(aumid, 'u"AyuGram.lnk"_q', 'u"Telegram.lnk"_q', count=2)
    replace(aumid, 'u"AyuGram Desktop/AyuGram.lnk"_q', 'u"Telegram Desktop/Telegram.lnk"_q')
    replace("Telegram/SourceFiles/platform/win/specific_win.cpp",
            '"\\\\AyuGram.lnk"', '"\\\\Telegram.lnk"', count=2)

    # настройки
    main = "Telegram/SourceFiles/ayu/ui/settings/settings_main.cpp"
    replace(main, 'QString("AyuGram Desktop v")', 'QString("Telegram Desktop v")')
    replace(main, '.title = rpl::single(QString("AyuGram")),', '.title = rpl::single(QString("Telegram")),')
    ayu = "Telegram/SourceFiles/ayu/ui/settings/settings_ayu.cpp"
    replace(ayu, '.title = u"AyuGram"_q,', '.title = u"Telegram"_q,')
    replace(ayu, 'return rpl::single(QString("AyuGram"));', 'return rpl::single(QString("Telegram"));')

    # заголовок выгрузки фильтров
    replace("Telegram/SourceFiles/ayu/features/filters/filters_utils.cpp",
            '"AyuGram Filters"', '"Telegram Filters"')

    # «неподдерживаемое сообщение»: AyuGram подменял в нём Telegram на AyuGram
    replace("Telegram/SourceFiles/history/history_item_helpers.cpp",
            '.replace("Telegram", "AyuGram")', '')

    # английские тексты по умолчанию (только значения, ключи не трогаем)
    rel = "Telegram/Resources/langs/lang.strings"
    text = read(rel)
    nl = eol(text)
    changed = 0
    out = []
    for line in text.split(nl):
        m = re.match(r'^("[^"]+"\s*=\s*")(.*)$', line)
        if m and "AyuGram" in m.group(2):
            line = m.group(1) + m.group(2).replace("AyuGram", "Telegram")
            changed += 1
        out.append(line)
    if changed == 0:
        errors.append(f"{rel}: не найдено строк со словом AyuGram")
    else:
        write(rel, nl.join(out))
        print(f"  {rel}: изменено строк: {changed}")

    # переводы, которые AyuGram скачивает при запуске
    insert_before(
        "Telegram/SourceFiles/ayu/ayu_lang.cpp",
        "Lang::GetInstance().resetValue(key.toUtf8());",
        '\t\tval = val.replace(qsl("AyuGram"), qsl("Telegram"), Qt::CaseInsensitive);',
    )


# ---------------------------------------- обходы для сборки библиотек

def fix_prepare():
    """Поправки скрипта подготовки библиотек. Не меняют программу.
    Если AyuGram уже исправил место у себя, поправка просто пропускается."""
    print("Подготовка библиотек:")
    rel = "Telegram/build/prepare/prepare.py"
    text = read(rel)
    # MSYS2 убрал пакет mingw-w64-x86_64-diffutils, есть только diffutils
    if "mingw-w64-x86_64-diffutils" in text:
        write(rel, text.replace("mingw-w64-x86_64-diffutils", "diffutils"))
        print("  mingw-w64-x86_64-diffutils -> diffutils")
    else:
        print("  поправка diffutils не нужна")

    # dump_syms (утилита для отчётов о падениях) требует ATL, которого нет
    # на машинах GitHub. Отчёты о падениях выключены, поэтому она не нужна.
    text = read(rel)
    nl = eol(text)
    pattern = re.compile(
        r"^\s+(cd tools\\\\windows\\\\dump_syms"
        r"|gyp dump_syms\.gyp --format=msvs"
        r"|msbuild -m dump_syms\.vcxproj .*)$"
    )
    lines = text.split(nl)
    kept = [l for l in lines if not pattern.match(l)]
    removed = len(lines) - len(kept)
    if removed:
        write(rel, nl.join(kept))
        print(f"  убрана сборка dump_syms ({removed} строки)")
    else:
        print("  поправка dump_syms не нужна")


if __name__ == "__main__":
    if sys.argv[1:] == ["prepare"]:
        fix_prepare()
        sys.exit(0)
    apply_icons()
    if not errors:
        apply_names()
    if errors:
        print("\nОШИБКА: не удалось применить изменения:")
        for e in errors:
            print("  - " + e)
        print("\nВероятно, AyuGram изменил эти места. Нужно поправить custom/apply.py.")
        sys.exit(1)
    print("\nГотово: все изменения применены.")
