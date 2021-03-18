<template>
  <div class="store">
    <div class="cur">
      <div>{{cur_dir}}</div>
      <button class="green return" @click="go_back" :disabled="dir.length <= 0">
        <i class="material-icons">keyboard_backspace</i>
      </button>
      <button class="yellow new-folder" @click="create_folder">
        <i class="material-icons">create_new_folder</i>
      </button>
    </div>
    <div class="main">
      <keep-alive>
        <router-view/>
      </keep-alive>
    </div>
    
    <!-- {{ $t('message') }} -->
    <div class="fs-menu" @click.stop="hide_menu"> 
      <i class="handle small material-icons">menu</i>
      <div class="fs-dropdown">
        <div class="all disabled" @click="switch_page($t('all'), 'all')"><i class="material-icons">store</i>{{$t('all')}}</div>
        <div class="image" @click="switch_page($t('image'), 'image')"><i class="material-icons">image</i>{{$t('image')}}</div>
        <div class="audio" @click="switch_page($t('audio'), 'audio')"><i class="material-icons">audiotrack</i>{{$t('audio')}}</div>
        <div class="video" @click="switch_page($t('video'), 'video')"><i class="material-icons">ondemand_video</i>{{$t('video')}}</div>
        
      </div>
      <div class="sort-dropdown">
        <div @click="set_sort_criteria(1)">
          {{$t('sort-by-time')}}
          <i v-if="order_by_time=='asc'" class="material-icons">arrow_upward</i>
          <i v-else-if="order_by_time=='desc'" class="material-icons">arrow_downward</i>
          <i v-else class="material-icons">swap_vert</i>
        </div>
        <div @click="set_sort_criteria(2)">
          {{$t('sort-by-name')}}
          <i v-if="order_by_name=='asc'" class="material-icons">arrow_upward</i>
          <i v-else-if="order_by_name=='desc'" class="material-icons">arrow_downward</i>
          <i v-else class="material-icons">swap_vert</i>
        </div>
        <div @click="set_sort_criteria(3)">
          {{$t('sort-by-type')}}
          <i v-if="order_by_type=='asc'" class="material-icons">arrow_upward</i>
          <i v-else-if="order_by_type=='desc'" class="material-icons">arrow_downward</i>
          <i v-else class="material-icons">swap_vert</i>
        </div>
        <div class="multi-sel" @click="switch_page($t('multi-sel'), 'multi-sel')">
          {{$t('multi-sel')}}
          <i class="material-icons">select_all</i>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import _ from 'lodash'
import moment from "moment";
import util from "@/common/util";

window.g = {
  files: []
};
export default {
  name: "store",
  created: function() {
    this.$root.$on("get_file_list", this.on_files_chg);
    this.$root.$on("enter_dir", this.on_enter_dir);
    this.$root.$on("create_dir", this.on_create_dir);
    this.$root.$on("refresh_file_list", this.on_refresh);
    this.$root.$on('move_to', this.move_to);
    this.$root.$on('back', this.back);
    this.$root.$on('confirm_move', this.confirm_move);
  },
  destroyed() {
    this.$root.$off("get_file_list", this.on_files_chg);
    this.$root.$off("enter_dir", this.on_enter_dir);
    this.$root.$off("create_dir", this.on_create_dir);
    this.$root.$off("refresh_file_list", this.on_refresh);
    this.$root.$off('move_to', this.move_to);
    this.$root.$off('back', this.back);
    this.$root.$off('confirm_move', this.confirm_move);
  },
  watch: {
    '$route' (to, from) {
      // console.log(`from=${JSON.stringify(from.name)}`)
      // console.log(`to=${JSON.stringify(to.name)}`)
      $(`.fs-menu > div > div`).removeClass('disabled')
      $(`.fs-dropdown > .${to.name}`).addClass('disabled')
    }
  },
  mounted() {
    this.draggie = new Draggabilly('.fs-menu', {
      containment: '.content', 
      handle: '.handle'
    });
    this.draggie.on( 'dragStart', ()=>{
      // $(".fs-menu").removeClass("is-open");
    });
    this.draggie.on( 'staticClick', ()=>{
      $(".fs-menu").toggleClass("is-open");
      // console.log('staticClick');
    });
    const ui_set = db.ui.findOne({})
    this.set_sort_criteria(ui_set.sort_type, ui_set.sort_asc, false);
    this.tm = setTimeout(()=>{
      this.on_refresh();
    }, 2500);
  },
  data() {
    return {
      refreshing: false,
      sort_type: 1,
      order_by_time: '',
      order_by_name: '',
      order_by_type: '',
      pending_f: [],
      original_page: 'all',
      dir: [],
      original_dir: [],
    };
  },
  computed: {
    cur_dir() {
      const p = this.dir.join("/");
      if (p) return `${p}/`;
      return ``;
    }
  },
  methods: {
    switch_page(sub, name){
      this.$root.$emit('sub_title_chg', sub);
      this.$router.replace({name: name})
    },
    set_sort_criteria(type, is_asc = 'asc', is_toggle = true) {
      switch(type){
        case 1:{
          if(is_toggle){
            if(this.order_by_time == 'asc') this.order_by_time = 'desc';
            else if(this.order_by_time == 'desc') this.order_by_time = 'asc';
            else this.order_by_time = 'asc';
            is_asc = this.order_by_time;
          } else {
            this.order_by_time = is_asc;
          }
          break;
        }
        case 2:{
          if(is_toggle){
            if(this.order_by_name == 'asc') this.order_by_name = 'desc';
            else this.order_by_name = 'asc';
            is_asc = this.order_by_name;
          } else {
            this.order_by_name = is_asc;
          }         
          break;
        }
        case 3:{
          if(is_toggle){
            if(this.order_by_type == 'asc') this.order_by_type = 'desc';
            else this.order_by_type = 'asc';
            is_asc = this.order_by_type;
          } else {
            this.order_by_type = is_asc;
          }           
          break;
        }
      }
      this.sort_type = type;
      db.ui.findAndUpdate({}, s => {
        s.sort_type = type;
        s.sort_asc = is_asc;
      });
      this.update_file_list();
    },
    sort(){
      switch(this.sort_type){
        case 1:{
          // cause desc does not work as expected, so reverse it
          // console.log(`order_by_time: ${this.order_by_time}`)
          g.files = _.orderBy(g.files, ['time']);
          if(this.order_by_time != 'asc') _.reverse(g.files);
          break;
        }
        case 2:{
          g.files = _.orderBy(g.files, ['name']); //always asc
          if(this.order_by_name != 'asc') _.reverse(g.files);
          break;
        }
        case 3:{
          g.files = _.orderBy(g.files, ['type']); //always asc
          if(this.order_by_type != 'asc') _.reverse(g.files);
          break;
        }
      }
    },
    back(){
      this.original_page = 'all'
      this.restore_before_move()
    },
    confirm_move(){
      let i = _.findIndex( g.files, f=> f.type == this.pending_f[0].type && f.name == this.pending_f[0].name );
      if(i >= 0) return util.show_alert_top_tm( this.$t('already-exist') )
      // actually move and restore previous dir
      for(let f of this.pending_f){
        const cmd = {
          cmd: "rename_file",
          path: f.path,
          new_name: this.cur_dir + f.name
        };
        ws.send(JSON.stringify(cmd));
      }
      this.restore_before_move()
    },
    restore_before_move(){
      this.dir = this.original_dir;
      this.switch_page(this.$t(this.original_page), this.original_page)
      this.on_refresh();
    },
    move_to(f){
      this.pending_f = [];
      this.original_dir = _.cloneDeep(this.dir);
      const cr = this.$router.currentRoute;
      // console.log(`cr.name=${cr.name}`)
      this.original_page = cr.name;
      if(f){
        this.$router.replace('folder', ()=>{
          this.pending_f.push(f);
        });
      } else {
        // 多选
        const sel_fs = _.filter(g.files, ff=>ff.sel);
        // console.log('多选移动： ' + JSON.stringify(sel_fs))
        if(sel_fs.length == 0){
          util.show_alert_top_tm( this.$t('no-selection') )
        } else {
          this.$router.replace('folder', ()=>{
            this.pending_f = sel_fs;
          });
        }
      }     
    },
    hide_menu(){
      // $(".fs-menu").removeClass("is-open");
    },
    on_create_dir(data) {
      if (data.ret == 0) {
        const n = util.get_name_from_path(data.path);
        util.show_info_center_tm(`${this.$t('create-folder')}【${n}】${this.$t('success')}`);
      } else {
        util.show_error_top(`${this.$t('create-folder')}${this.$t('fail')}: ${data.msg}`);
      }
    },
    create_folder() {
      let name = prompt(this.$t('new-folder-name'), this.$t('new-folder'));
      if(!name) return; //this is cancelled?
      name = name.replace(/[\n\r]/gm, "");
      if(!name) return util.show_alert_top_tm(`${this.$t('new-folder-name')}${this.$t('can-not-be-empty')}`)
      let i = _.findIndex( g.files, f=> f.type == 'dir' && f.name == name );
      if(i >= 0) return util.show_alert_top_tm(this.$t('already-exist'))
      if (name) {
        const cmd = {
          cmd: "create_dir",
          path: `${this.cur_dir}${name}`
        };
        ws.send(JSON.stringify(cmd));
      }
    },
    go_back() {
      this.dir.pop();
      this.on_refresh();
    },
    on_enter_dir(name) {
      if(this.refreshing) return;
      this.dir.push(name);
      this.on_refresh();
    },

    update_file_list(files){
      // maybe empty string, because Boost property-tree represent empty array as empty string
      if(files != null) g.files = files || [];
      // console.log('before sort: '+JSON.stringify(g.files))
      this.sort();
      // console.log('after sort: '+JSON.stringify(g.files))
      this.$root.$emit("update_file_list", g.files);
    },
    // ws onopen emit
    on_files_chg(data) {
      clearTimeout(this.tm);
      // console.log('on_files_chg: '+JSON.stringify(data) )
      this.update_file_list(data.files);
    },
    async on_refresh() {
      console.log(`on_refresh ${this.cur_dir}`);
      this.refreshing = true;
      try {
        const res = await util.post_local("get_files", {
          path: this.cur_dir
        });
        // console.log(JSON.stringify(res));
        this.update_file_list(res.files);
      } catch (err) {
        console.log(`refresh ${this.cur_dir} failed: ${JSON.stringify(err)}`);
        this.dir.pop();
      }
      this.refreshing = false;
    },
    
  }
};
</script>
<style scoped >
.store{
  overflow: hidden;
  position: relative;
}
.handle{
  padding: 0.35em;
}
.handle.material-icons{
  vertical-align: sub; 
  pointer-events: initial;
}
.fs-menu{
  position: absolute;
  /* must use left/top, or it will stretch */
  left: 55%;
  top: 40%;
  border-radius: 2em;
  background-color: rgb(230, 227, 227);
}
.fs-dropdown{
  background-color: lightgray;
  display: none;
  white-space: nowrap;
  position: absolute;
  right: 50%;
  top: 80%;
  margin: 0;
}
.sort-dropdown{
  /* font-size: 1.06em; */
  background-color: lightgray;
  display: none;
  white-space: nowrap;
  position: absolute;
  top: 80%;
  margin: 0;
  left: 50%;
}
.fs-dropdown > div + div, .sort-dropdown > div + div{
  border-top: 1px outset;
}
.fs-dropdown > div, .sort-dropdown > div {
  /* width: 100%; */
  text-align: center;
  padding: 0.8em 1em;
  font-weight: normal;
}
.fs-menu.is-open > div {
  display: block;
}
.cur > div:first-child {
  /* border: 1px solid red; */
  flex: 1;
}
.cur {
  /* position: fixed;
  width: 100%;
  top: 2rem; */
  align-items: center;
  display: flex;
  border: 1px solid;
  text-align: left;
  background-color: rgb(130, 233, 247);
  z-index: 7;
}
.main {
  height: calc(100vh - 6.5rem);
  overflow-y: auto;
}

.return,
.new-folder {
  border: none;
  outline: none;
  background-color: transparent;
  font-weight: 900;
  /* border-radius: 1em; */
  padding: 0.7em;
  margin-left: auto;
}
button:disabled {
  /* background-color: #ccc; */
  color: grey;
}
div.disabled
{
  pointer-events: none;
  /* for "disabled" effect */
  opacity: 0.5;
  background: #CCC;
}

</style>
