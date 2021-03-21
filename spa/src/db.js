import Loki from 'lokijs';
import LokiIndexedAdapter from 'lokijs/src/loki-indexed-adapter';
const idbAdapter = new LokiIndexedAdapter();

import util from "@/common/util";
let db;

function populate_default() {
    if (!db.ui.findOne({})) {
        db.ui.insert({
            sort_type: 1,
            sort_asc: false,
            audio_loop_type: '1'
        });
    }
    if (!db.svr.findOne({})) {
        db.svr.insert({
            http_port: 57000,
            socks_port: 58000,
        });
    }
    if (!db.ss.findOne({})) {
        db.ss.insert({
            addr: '139.155.50.166:57000',
            enabled: false
        });
    }
    
}

//export promise?
export default new Promise((resolve, reject) => {
    if (db) {
        resolve(db);
    } else {
        let mgrDB = new Loki("pyStore.db", {
            adapter: idbAdapter,
            autoload: true,
            autoloadCallback: () => {
                db = {
                    user: mgrDB.getCollection("user") ? mgrDB.getCollection("user") : mgrDB.addCollection("user"),
                    friends: mgrDB.getCollection("friends") ? mgrDB.getCollection("friends") : mgrDB.addCollection("friends"),
                    blacklist: mgrDB.getCollection("blacklist") ? mgrDB.getCollection("blacklist") : mgrDB.addCollection("blacklist"),
                    ui: mgrDB.getCollection("ui") ? mgrDB.getCollection("ui") : mgrDB.addCollection("ui"),
                    svr: mgrDB.getCollection("svr") ? mgrDB.getCollection("svr") : mgrDB.addCollection("svr"),
                    ss: mgrDB.getCollection("ss") ? mgrDB.getCollection("ss") : mgrDB.addCollection("ss"),
                    // id, nickname, content, type, [img, text], dt, dir[0, 1]. where id is chat target id
                    peer_chat_log: mgrDB.getCollection("peer_chat_log") ? mgrDB.getCollection("peer_chat_log") : mgrDB.addCollection("peer_chat_log"),
                    // id is always refer to source
                    nearby_chat_log: mgrDB.getCollection("nearby_chat_log") ? mgrDB.getCollection("nearby_chat_log") : mgrDB.addCollection("nearby_chat_log"),
                    world_chat_log: mgrDB.getCollection("world_chat_log") ? mgrDB.getCollection("world_chat_log") : mgrDB.addCollection("world_chat_log"),
                }
                populate_default();

                resolve(db);
            },
            autosave: true,
            autosaveInterval: 1000
        });
    }
})