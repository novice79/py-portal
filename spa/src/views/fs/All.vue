<template>
  <div class="All" @click.prevent="close_menu">
    
    <!-- {{ $t('message') }} -->
    <div class="fi" :key="f.name" v-for="f in files" @click="open_file_or_folder(f)">
      <div v-if="f.type=='dir'">
        <i class="small yellow material-icons">folder</i>
      </div>
      <div v-else-if="f.type.includes('image/')">
        <i class="small material-icons">image</i>
      </div>
      <div v-else-if="f.type.includes('audio/')">
        <i class="small material-icons">audiotrack</i>
      </div>
      <div v-else-if="f.type.includes('video/')">
        <i class="small material-icons">ondemand_video</i>
      </div>
      <div v-else>
        <i class="small material-icons">insert_drive_file</i>
      </div>
      <div class="file-desc">
        <div class="fn">{{f.name}}</div>
        <div class="file-time">
          <div>{{f.time}}</div>
          <div class="file-size" v-if="f.type != 'dir'">{{formatFileSize(f.size)}}</div>
        </div>
        <div v-if="f.type!='dir' && is_shown(f.name)">
          <img v-if="f.type.includes('image/')" :src="file_url(f.name)" />
          <audio v-else-if="f.type.includes('audio/')" :src="file_url(f.name)" controls  @click.stop="1"/>
          <video
            v-else-if="f.type.includes('video/')"
            :src="file_url(f.name)"
            controls="controls" @click.stop="1"
          />
          <a v-else :href="file_url(f.name)" target="_blank" @click.stop="1">{{$t('open-file')}}</a>
        </div>
      </div>
      <div class="op-menu" @click.stop="toggle_menu">
        <div class="op-btn">&#8942;</div>
        <div class="op-dropdown">
          <div class="row">
            <div @click="del_file(f)">{{$t('delete')}}</div>
            <div @click="rename_file(f)">{{$t('rename-file')}}</div>
            <div @click="move_to(f.name)">{{$t('move')}}</div>
          </div> 
          <div class="row">
            <div v-if="f.type!='dir'">
              <a :href="file_url(f.name)" :download="f.name"> 下载 </a> 
            </div>
            <div v-if="is_zip_file(f)" @click="uncompress(f)">
              解压
            </div>          
            <div v-if="is_srt_file(f)" @click="to_vtt(f)">
              转vtt
            </div> 
          </div> 

        </div>
      </div>
    </div>
    <div class="pad-bottom"></div>
  </div>
</template>

<script>
import { mapGetters, mapActions } from 'vuex'
import util from "@/common/util";

export default {
  name: "All",
  created: function() {

  },
  destroyed() {

  },
  mounted() {

  },
  data() {
    return {
      shown: {}
    };
  },
  computed: {
    ...mapGetters({
      files: 'all',
    }),
    ...mapGetters([
      'file_path',
      'file_url'
    ])
  },
  methods: {
    ...mapActions([
      'enter',
      'back',
      'delete',
      'rename',
      'create_dir',
      'move_to',
      'confirm_move'
    ]),
    is_shown(fn){
      return this.shown[fn]
    },
    async to_vtt(f) {
      try {
        const res = await util.post_local("ffmpeg", { code: 1, srt: f.name }, 1);
        if(res.ret == 0){
          util.show_alert_top_tm(`[${f.name}]转vtt成功`);
        } else{
          util.show_alert_top_tm(`[${f.name}]转vtt失败`);
        }
      } catch (err) {
        console.log(`${f.name}转vtt failed: ${JSON.stringify(err)}`);
        util.show_error_top(`[${f.name}]转vtt失败`);
      }
    },
    is_srt_file(f){
      return f.name.substr(f.name.length - 4) === '.srt';
    },
    is_zip_file(f){
      return f.type.includes('/zip') || f.type.includes('/x-7z') || f.type.includes('/x-rar');
    },
    async uncompress(f) {
      try {
        const res = await util.post_local("uncompress", { file: f.name }, 1);
        if(res.ret == 0){
          util.show_alert_top_tm(`解压文件[${f.name}]成功`);
        } else{
          util.show_alert_top_tm(`解压文件[${f.name}]失败`);
        }
      } catch (err) {
        console.log(`uncompress ${this.cur_dir} failed: ${JSON.stringify(err)}`);
        util.show_error_top(`解压文件[${f.name}]失败`);
      }

    },
    open_file_or_folder(f) {
      if (f.type == 'dir') {
        this.enter(f.name);
      } else {
        this.shown = { ...this.shown, [f.name]: !this.shown[f.name] }
      }
    },
    close_menu() {
      $(".op-menu").removeClass("is-open");
    },
    toggle_menu(e) {
      const isOpen = e.currentTarget.classList.contains("is-open");
      this.close_menu();
      !isOpen && e.currentTarget.classList.toggle("is-open");
    },

    rename_file(f) {
      let new_name = prompt(this.$t('new-file-name'), f.name);
      if(new_name) new_name = new_name.replace(/[\n\r]/gm, "");
      let i = _.findIndex( this.files, ff=> ff.type == f.type && ff.name == new_name );
      if(i >= 0) return util.show_alert_top_tm(`${this.$t('same-file')}${this.$t('already-exist')}`)
      if (new_name && new_name != f.name) {
        this.rename({old_names:this.file_path(f.name), new_name: this.file_path(new_name)}) 
      }
    },
    del_file(f) {
      const r = confirm( `${this.$t('confirm-del')}[${f.name}]？`)
      if(r){
        this.delete(f.name)
      }
    },
    formatFileSize(bytes, decimalPoint) {
      return util.formatFileSize(bytes, decimalPoint)
    }
  }
};
</script>
<style scoped >
.All {
  max-width: 100%;
}
.file-desc {
  flex: 1;
  margin: 0 0.4em;
  /* border: 1px solid blue; */
}
.fn{
  word-break: break-all;
}
.op-menu {
  position: relative;
}
.op-btn {
  font-size: 1.5em;
  margin: 0.1em 0.5em;
}
.op-dropdown {
  background-color: lightgray;
  display: none;
  white-space: nowrap;
  position: absolute;
  right: 2.3em;
  top: 1.2em;
  margin: 0;
  z-index: 79;
}
/* \0+hexcode */
.op-dropdown::after {
    content: "\021A2";
    position: absolute;
    font-size: xx-large;
    right: -0.8em;
    top: -0.79em;
    color: green;
}
.op-dropdown > .row + .row{
  border-left: 1px outset;
}
.row > div + div {
  border-top: 1px outset;
}
.row > div {
  /* width: 100%; */
  text-align: center;
  padding: 0.8em 1em;
  font-weight: normal;
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

.op-menu.is-open .op-dropdown {
  /* display: block; */
  display: flex;
}
.file-size,
.op-menu {
  margin-left: auto;
}
.file-time {
  display: flex;
  font-size: 0.7rem;
}

.fi {
  display: flex;
  align-items: center;
  text-align: left;
  background-color: rgb(177, 250, 250);
  margin: 0.3em 0;
  border: 2px inset black;
  max-width: 100%;
}
button:disabled {
  /* background-color: #ccc; */
  color: grey;
}
/* no effect */
/* audio, video{
  pointer-events: none;
} */
audio,
video,
img {
  width: 100%;
}
</style>
