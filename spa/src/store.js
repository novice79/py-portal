
import Vue from 'vue'
import Vuex from 'vuex'
Vue.use(Vuex)
import _ from 'lodash'
import util from "@/common/util"

let pending_f = [], original_page = 'all', original_dir = [];

export default new Vuex.Store({
    state: {
        // 0 = no sort
        sort_type: 1,
        // false: asc or true: desc
        order_type: false,
        requesting: false,
        dirs: [],
        files: []
    },
    getters: {
        path: ({ dirs }) => dirs.join("/"),
        file_path: (state, { path }) => (file_name) => {
            return path ? `${path}/${file_name}` : `${file_name}`
        },
        file_url: (state, { path }) => (file_name) => {
            return path
                ? `${util.store_url()}/${path}/${file_name}`
                : `${util.store_url()}/${file_name}`
        },
        all: ({ sort_type, order_type, files }) => {
            switch (sort_type) {
                case 1: {
                    // cause desc does not work as expected, so reverse it
                    files = _.orderBy(files, ['time']);
                    break;
                }
                case 2: {
                    files = _.orderBy(files, ['name']);
                    break;
                }
                case 3: {
                    files = _.orderBy(files, ['type']);
                    break;
                }
            }
            return sort_type && order_type ? _.reverse(files) : files
        },
        images: (state, { all }) =>
            all.filter(f => f.type.includes('image/'))
        ,
        audios: (state, { all }) =>
            all.filter(f => f.type.includes('audio/'))
        ,
        videos: (state, { all }) =>
            all.filter(f => f.type.includes('video/'))
        ,
        dirs: (state, { all }) =>
            all.filter(f => f.type == 'dir')
        ,
    },
    mutations: {
        change_dir(state, { dirs, files }) {
            state.dirs = [...dirs]
            state.files = [...files]
        },
        set_requesting(state, req) {
            state.requesting = req
        },
        set_sort_type(state, type) {
            state.sort_type = type
        },
        set_order_type(state, type) {
            state.order_type = type
        },
    },
    actions: {
        async refresh({ state, getters, commit, dispatch }, path) {
            if (path && getters.path != path) return;
            commit('set_requesting', true)
            try {
                const res = await util.post_local("get_files", {
                    path: getters.path
                });
                res.dirs = state.dirs;
                commit('change_dir', res)
            } catch (err) {
                console.log(`refresh ${getters.path} failed: ${JSON.stringify(err)}`);
            }
            commit('set_requesting', false)
        },
        async enter({ state, getters, commit, dispatch }, dir) {
            const pending_d = _.cloneDeep(state.dirs);
            pending_d.push(dir)
            commit('set_requesting', true)
            try {
                // res contain files array
                const res = await util.post_local("get_files", {
                    path: pending_d.join("/")
                });
                res.dirs = pending_d;
                commit('change_dir', res)
            } catch (err) {
                console.log(`enter ${pending_d.join("/")} failed: ${JSON.stringify(err)}`);
            }
            commit('set_requesting', false)
        },
        async back({ state, getters, commit, dispatch }) {
            const pending_d = _.cloneDeep(state.dirs);
            pending_d.pop()
            commit('set_requesting', true)
            try {
                // res contain files array
                const res = await util.post_local("get_files", {
                    path: pending_d.join("/")
                });
                res.dirs = pending_d;
                commit('change_dir', res)
            } catch (err) {
                console.log(`enter ${pending_d.join("/")} failed: ${JSON.stringify(err)}`);
            }
            commit('set_requesting', false)
        },
        // file operations
        async delete({ state, getters: { path }, commit, dispatch }, fns) {
            if( !Array.isArray(fns) ) fns = [fns];
            // suppose fns just names, so add in rel path to it
            commit('set_requesting', true)
            try {
                const res = await util.post_local("file_op/delete", {
                    files: fns.map(fn => `${path}/${fn}`)
                });
                if (res.ret == 0) {
                    util.show_success_top(`成功删除${res.count}个文件`)
                } else {
                    util.show_error_top(`删除文件失败:${res.msg}`)
                }

            } catch (err) {
                console.log(`delete ${fns} failed: ${JSON.stringify(err)}`);
            }
            commit('set_requesting', false)
        },
        async rename({ state, getters, commit, dispatch }, {old_names, new_name}) {
            // suppose parameters already with relative path
            if( !Array.isArray(old_names) ) old_names = [old_names];
            commit('set_requesting', true)
            try {
                console.log(`old_names=${JSON.stringify(old_names)}, new_name=${new_name}`)
                const res = await util.post_local("file_op/rename", {
                    old_names,
                    new_name
                });
                if (res.ret == 0) {
                    util.show_success_top(`移动至${new_name}成功`)
                } else {
                    util.show_error_top(`移动至${new_name}失败`)
                }

            } catch (err) {
                console.log(`move ${old_names} failed: ${JSON.stringify(err)}`);
            }
            commit('set_requesting', false)
        },
        async create_dir({ state, getters: { path }, commit, dispatch }, name) {
            // suppose name just name, so add in rel path to it
            commit('set_requesting', true)
            try {
                const res = await util.post_local("file_op/create_dir", {
                    path: `${path}/${name}`
                });
                if (res.ret == 0) {
                    util.show_success_top(`创建${name}成功`)
                } else {
                    util.show_error_top(`创建${name}失败`)
                }

            } catch (err) {
                console.log(`create dir ${name} failed: ${JSON.stringify(err)}`);
            }
            commit('set_requesting', false)
        },
        move_to({ state: {dirs}, getters: {path}, commit, dispatch  }, sel_f) {
            // sel_f should be a array of selected files to be moved
            if( !Array.isArray(sel_f) ) sel_f = [sel_f];
            original_dir = _.cloneDeep(dirs);
            // const cr = router.currentRoute;
            // // console.log(`cr.name=${cr.name}`)
            // original_page = cr.name;
            // always return to all page
            original_page = 'all'
            router.replace('folder', () => {
                pending_f = sel_f.map(fn=>`${path}/${fn}`)
            });
        },
        async confirm_move({ state, getters, commit, dispatch }) {
            // todo:
            let i = _.findIndex(state.files, f => f.type == pending_f[0].type && f.name == pending_f[0].name);
            if (i >= 0) return util.show_alert_top_tm(i18n.t('already-exist'))
            // actually move and restore previous dir
            await dispatch('rename', {old_names: pending_f, new_name: getters.path})
            dispatch('restore_before_move')
        },
        restore_before_move({ state, getters, commit, dispatch }) {
            // restore_before_move
            state.dirs = original_dir;
            util.switch_page(i18n.t(original_page), original_page)
            dispatch('refresh')
        },
    }
})