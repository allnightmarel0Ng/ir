import os
import json
import time
import yaml
import argparse
from datetime import datetime
import requests
from collections import deque
from pymongo import MongoClient
from urllib.parse import urljoin

CONFIG = {
    'db': {
        'host': 'localhost',
        'port': 27017,
        'database': 'wiki_corpus',
        'collection': 'pages'
    },
    'logic': {
        'delay_between_pages': 0.05,
        'save_dir': 'documents/',
        'max_pages': 50000,
        'batch_size': 2000,
        'api_delay': 0.1,
        'page_delay': 0.05,
        'min_text_chars': 300
    }
}

START_CATEGORIES = [
    "Категория:История",
    "Категория:Всемирная история",
    "Категория:История по странам",
    "Категория:История по периодам",
    "Категория:Исторические события",
    "Категория:Исторические эпохи",
    "Категория:Исторические личности",
    "Категория:Войны",
    "Категория:Сражения",
    "Категория:Революции",
    "Категория:Империи",
    "Категория:Государства в истории",
    "Категория:Историческая хронология",
]

API_URL = "https://ru.wikipedia.org/w/api.php"

session = requests.Session()
session.headers.update({
    "User-Agent": "MAI-IR-TransportCorpus/1.0 (student project; contact: example@example.com)"
})

def load_config(config_path):
    with open(config_path, 'r', encoding='utf-8') as f:
        config = yaml.safe_load(f)

    if 'db' in config:
        CONFIG['db'].update(config['db'])
    if 'logic' in config:
        CONFIG['logic'].update(config['logic'])
    
    return CONFIG

def init_mongodb():
    db_config = CONFIG['db']
    client = MongoClient(
        host=db_config.get('host', 'localhost'),
        port=db_config.get('port', 27017)
    )
    db = client[db_config.get('database', 'wiki_corpus')]
    collection = db[db_config.get('collection', 'pages')]
    return collection

def fetch_category_members(cat, cmcontinue=None):
    params = {
        "action": "query",
        "list": "categorymembers",
        "cmtitle": cat,
        "cmtype": "page|subcat",
        "cmlimit": "max",
        "format": "json",
    }
    if cmcontinue:
        params["cmcontinue"] = cmcontinue

    r = session.get(API_URL, params=params, timeout=30)
    r.raise_for_status()
    data = r.json()

    members = data["query"]["categorymembers"]
    next_token = data.get("continue", {}).get("cmcontinue")
    return members, next_token

def get_plain_text(pageid):
    params = {
        "action": "query",
        "pageids": pageid,
        "prop": "extracts",
        "explaintext": True,
        "format": "json",
    }
    r = session.get(API_URL, params=params, timeout=40)
    r.raise_for_status()
    pages = r.json()["query"]["pages"]
    p = pages[str(pageid)]
    return p.get("title", ""), p.get("extract", "")

def collect_pages():
    logic_config = CONFIG['logic']
    save_dir = logic_config['save_dir']
    PAGES_LIST_PATH = os.path.join(save_dir, "pages_list.json")
    
    queue = deque(START_CATEGORIES)
    visited_categories = set()
    pages = {}
    
    max_pages = logic_config.get('max_pages', 50000)
    api_delay = logic_config.get('api_delay', 0.1)

    while queue and len(pages) < max_pages:
        cat = queue.popleft()
        if cat in visited_categories:
            continue
        visited_categories.add(cat)

        print(f"[CAT] Обрабатываю категорию: {cat}")
        cmc = None

        while True:
            try:
                members, cmc = fetch_category_members(cat, cmc)
            except Exception as e:
                print(f"[ERR] Ошибка при запросе категории {cat}: {e}")
                break

            for m in members:
                ns = m["ns"]
                title = m["title"]
                pageid = m["pageid"]

                if ns == 14:
                    queue.append(title)
                elif ns == 0:
                    if pageid not in pages:
                        pages[pageid] = title
                        if len(pages) >= max_pages:
                            break

            print(f"    найдено статей: {len(pages)}")

            if len(pages) >= max_pages:
                break
            if not cmc:
                break

            time.sleep(api_delay)

    print(f"[CAT] Сбор страниц завершён. Итоговое количество статей: {len(pages)}")
    return pages

def save_pages_list(pages_dict):
    logic_config = CONFIG['logic']
    save_dir = logic_config['save_dir']
    os.makedirs(save_dir, exist_ok=True)
    
    PAGES_LIST_PATH = os.path.join(save_dir, "pages_list.json")
    pages_list = [{"pageid": pid, "title": title} for pid, title in pages_dict.items()]
    with open(PAGES_LIST_PATH, "w", encoding="utf-8") as f:
        json.dump(pages_list, f, ensure_ascii=False, indent=2)
    print(f"[INFO] Список страниц сохранён в {PAGES_LIST_PATH}")

def load_pages_list():
    logic_config = CONFIG['logic']
    save_dir = logic_config['save_dir']
    PAGES_LIST_PATH = os.path.join(save_dir, "pages_list.json")
    
    with open(PAGES_LIST_PATH, "r", encoding="utf-8") as f:
        pages_list = json.load(f)
    print(f"[INFO] Загружен список страниц из {PAGES_LIST_PATH}, всего: {len(pages_list)}")
    return pages_list

def load_existing_documents_from_files(collection):
    logic_config = CONFIG['logic']
    save_dir = logic_config['save_dir']
    
    if not os.path.exists(save_dir):
        print(f"[INFO] Папка {save_dir} не существует, пропускаем загрузку существующих документов")
        return 0
    
    batch_files = [
        name for name in os.listdir(save_dir)
        if name.startswith("batch_") and name.endswith(".jsonl")
    ]
    
    if not batch_files:
        print("[INFO] Нет существующих batch файлов для загрузки")
        return 0
    
    processed_count = 0
    for batch_file in batch_files:
        batch_path = os.path.join(save_dir, batch_file)
        print(f"[INFO] Загружаем документы из {batch_file}")
        
        with open(batch_path, "r", encoding="utf-8") as f:
            for line in f:
                try:
                    doc = json.loads(line.strip())
                    pageid = doc["pageid"]
                    title = doc["title"]
                    text = doc["text"]

                    url = f"https://ru.wikipedia.org/?curid={pageid}"
                    
                    mongo_doc = {
                        "url": url,
                        "text": text,
                        "title": title,
                        "created_at": int(time.time()),
                        "pageid": pageid,
                        "source": "ru.wikipedia.org"
                    }
                    
                    collection.update_one(
                        {"pageid": pageid},
                        {"$set": mongo_doc},
                        upsert=True
                    )
                    
                    processed_count += 1
                    if processed_count % 1000 == 0:
                        print(f"[INFO] Загружено {processed_count} документов в MongoDB")
                        
                except Exception as e:
                    print(f"[WARN] Ошибка при обработке документа из {batch_file}: {e}")
                    continue
    
    print(f"[INFO] Всего загружено {processed_count} документов из файлов в MongoDB")
    return processed_count

def download_texts(pages_list, collection):
    logic_config = CONFIG['logic']
    save_dir = logic_config['save_dir']
    os.makedirs(save_dir, exist_ok=True)
    
    PAGES_LIST_PATH = os.path.join(save_dir, "pages_list.json")
    min_text_chars = logic_config.get('min_text_chars', 300)
    batch_size = logic_config.get('batch_size', 2000)
    page_delay = logic_config.get('page_delay', 0.05)
    
    if os.path.exists(PAGES_LIST_PATH):
        with open(PAGES_LIST_PATH, "r", encoding="utf-8") as f:
            saved_pages = json.load(f)
        
        last_processed_pageid = None
        if saved_pages:
            last_doc = collection.find_one(sort=[("created_at", -1)])
            if last_doc:
                last_processed_pageid = last_doc.get("pageid")
    
    existing_batches = [
        name for name in os.listdir(save_dir)
        if name.startswith("batch_") and name.endswith(".jsonl")
    ]
    
    if existing_batches:
        max_batch = max(int(name[6:10]) for name in existing_batches)
        batch_id = max_batch + 1
    else:
        batch_id = 1
    
    current_batch_path = os.path.join(save_dir, f"batch_{batch_id:04}.jsonl")
    f = open(current_batch_path, "a", encoding="utf-8")
    counter_in_batch = 0
    
    total_pages = len(pages_list)
    processed_count = 0
    skipped_count = 0
    
    for idx, page in enumerate(pages_list):
        pageid = page["pageid"]
        
        if last_processed_pageid and pageid <= last_processed_pageid:
            continue
        
        existing_doc = collection.find_one({"pageid": pageid})
        if existing_doc:
            print(f"[INFO] Страница {pageid} уже есть в базе, пропускаем")
            skipped_count += 1
            continue
        
        try:
            title, text = get_plain_text(pageid)
        except Exception as e:
            print(f"[ERR] pageid={pageid}: {e}")
            continue

        if not text or len(text) < min_text_chars:
            print(f"[INFO] Текст страницы {pageid} слишком короткий, пропускаем")
            continue

        file_doc = {"pageid": pageid, "title": title, "text": text}
        f.write(json.dumps(file_doc, ensure_ascii=False) + "\n")
        counter_in_batch += 1
        processed_count += 1

        url = f"https://ru.wikipedia.org/?curid={pageid}"
        
        mongo_doc = {
            "url": url,
            "text": text,
            "title": title,
            "created_at": int(time.time()),
            "pageid": pageid,
            "source": "ru.wikipedia.org"
        }
        
        try:
            collection.insert_one(mongo_doc)
        except Exception as e:
            print(f"[ERR] Ошибка при сохранении в MongoDB pageid={pageid}: {e}")
        
        if counter_in_batch >= batch_size:
            f.close()
            print(f"[INFO] Batch {batch_id} заполнен, создаем новый")
            batch_id += 1
            current_batch_path = os.path.join(save_dir, f"batch_{batch_id:04}.jsonl")
            f = open(current_batch_path, "a", encoding="utf-8")
            counter_in_batch = 0

        if (processed_count + skipped_count) % 100 == 0:
            print(f"[DL] Обработано страниц: {processed_count + skipped_count} / {total_pages} (скачано: {processed_count}, пропущено: {skipped_count})")

        time.sleep(page_delay)
    
    f.close()
    print(f"[DL] Скачивание текстов завершено. Всего обработано: {processed_count + skipped_count}, скачано: {processed_count}")

def main():
    parser = argparse.ArgumentParser(description='Поисковый робот для Wikipedia')
    parser.add_argument('config_path', type=str, help='Путь к YAML конфигурационному файлу')
    args = parser.parse_args()
    
    load_config(args.config_path)
    print("[INFO] Конфигурация загружена")
    
    collection = init_mongodb()
    print("[INFO] Подключение к MongoDB установлено")
    
    load_existing_documents_from_files(collection)
    
    logic_config = CONFIG['logic']
    save_dir = logic_config['save_dir']
    PAGES_LIST_PATH = os.path.join(save_dir, "pages_list.json")
    
    need_collect = True
    if os.path.exists(PAGES_LIST_PATH):
        try:
            pages_list_tmp = load_pages_list()
            if len(pages_list_tmp) > 0:
                need_collect = False
        except:
            need_collect = True
    
    if need_collect:
        pages_dict = collect_pages()
        save_pages_list(pages_dict)

    pages_list = load_pages_list()
    
    if len(pages_list) == 0:
        print("[FATAL] В списке страниц 0 элементов. Возможно, доступ к API Википедии блокируется провайдером или сетью.")
        return
    
    download_texts(pages_list, collection)
    
    print("ГОТОВО. Корпус сохранён в MongoDB и в папку:", save_dir)
    print(f"Коллекция MongoDB: {CONFIG['db']['database']}.{CONFIG['db']['collection']}")

if __name__ == "__main__":
    main()