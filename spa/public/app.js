// This file is utf-8 encoding
// steps for creating express+sqlite nodejs server demo (run on android)
// step 1: npm i express sqlite3
// step 2: put following content into app.js
const path = require('path');
const express = require('express')
const app = express()
const sqlite3 = require('sqlite3').verbose();
// this is db file name, so if you need to backup database, download db file from
// http://android-wifi-ip:57001/db
const db = new sqlite3.Database(path.join(__dirname, 'db'));
db.serialize( ()=> {
  db.run("create table if not exists test (count int)");
  db.get("select count(*) as rc from test", (err, row) => {
    if(row.rc > 0 ){
      app.listen(3000)
    } else {
      db.run("insert into test values(0)", ()=>{
        app.listen(3000)
      });
    }
  })
});
// optionally, put index.html into public folder for nodejs spa
app.use(express.static(path.join(__dirname, 'public')))
app.get('/express', (req, res) => {
  db.run("update test set count=count+1 where rowid=1", ()=>{
    db.get("select count from test where rowid=1", (err, row) => {
      res.send(`expressjs server say: hello world! [${row.count} times]`)
    })
  });
})
// optionally, create index.html in project root folder for c++ server(listen on port: 57001 default) spa
// now you has it:
// ├── app.js
// ├── index.html
// ├── node_modules
// └── public
//     └── index.html
// step 3: compress above files to a [ zip || rar || 7z ] file
// step 4: upload this file through http://android-wifi-ip:57000/uphome to your device
// and then: you can view these
// http://android-wifi-ip:57001           ---project root index.html
// http://android-wifi-ip:3000            ---project express public foler index.html
// http://android-wifi-ip:3000/express    ---express update and return sqlite DB data