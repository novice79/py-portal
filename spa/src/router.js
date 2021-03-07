import Vue from 'vue';
import Router from 'vue-router';
import Store from '@/views/Store.vue';

import All from '@/views/fs/All.vue';
import Folder from '@/views/fs/Folder.vue';
import Image from '@/views/fs/Image.vue';
import Audio from '@/views/fs/Audio.vue';
import Video from '@/views/fs/Video.vue';
import MultiSel from '@/views/fs/MultiSel.vue';

import Upload from '@/views/Upload.vue'
import UpHome from '@/views/UpHome.vue'

import Sss from '@/views/Sss.vue';

Vue.use(Router);

export default new Router({
  // mode: 'history',
  // mode: 'hash',
  routes: [
    {
      path: '/',
      name: 'store',
      component: Store,
      children: [
        { path: '', name: 'all', component: All },
        { path: 'folder', name: 'folder', component: Folder },
        { path: 'image', name: 'image', component: Image },
        { path: 'audio', name: 'audio', component: Audio },
        { path: 'video', name: 'video', component: Video },
        { path: 'multi-sel', name: 'multi-sel', component: MultiSel },
        // ...other sub routes
      ]
    },
    {
      path: '/outer',
      name: 'outer',
      component: () => import('./views/Outer.vue')
    },
    {
      path: '/help',
      name: 'help',
      component: () => import(/* webpackChunkName: "help" */ './views/Help.vue')
    },
    {
      path: '/help-en',
      name: 'help-en',
      component: () => import(/* webpackChunkName: "help-en" */ './views/Help_en.vue')
    },
    {
      path: '/upload',
      name: 'upload',
      component: Upload
    },
    {
      path: '/uphome',
      name: 'uphome',
      component: UpHome
    },
    { path: '*', redirect: '/' }
  ]
});
