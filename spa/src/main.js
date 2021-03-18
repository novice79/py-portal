import Vue from 'vue'

import App from '@/App.vue'
import router from '@/router'
import store from '@/store'
import i18n from '@/i18n'
import "@/assets/animate.css"
window.$ = require('jquery')
import Draggabilly from 'draggabilly'
window.Draggabilly = Draggabilly;

import adb from "@/db";
Vue.config.productionTip = false
Vue.prototype.window = window;
window.i18n = i18n;
window.store = store;
store.dispatch('back')
console.log(`navigator.language = ${navigator.language}`);
if(navigator.language === 'zh-CN'){
  i18n.locale = 'zh'
}
// "navigator.language = zh-CN"
// "navigator.language = en-US"
adb.then( db=>{
  window.db = db;
  new Vue({
    router,
    store,
    i18n,
    render: h => h(App)
  }).$mount('#app'); 
})


