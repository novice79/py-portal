<template>
  <div class="video">
    <div class="fi" :key="f.name" v-for="f in files" @click="play_video(f, $event)" >
      <div class="file-desc">
        <div>{{f.name}}</div>
        <div class="file-time">
          <div>{{f.time}}</div>
          <div>{{formatFileSize(f.size)}}</div>
          <div>{{durations[f.name]}}</div>
          <div style="cursor:pointer;" @click.stop.prevent="get_info(f, $event)">
            <i class=" material-icons">info</i>
          </div>         
        </div>
        <div class="media-info">
          <div v-for="(s, i) in streams[f.path]" :key="i">
            <div v-if="s.progress && s.progress != '100.00%'" class="progressbar">
              <div v-bind:style="{width: s.progress}"></div>
              <div class="cap">{{`${s.progress}[${s.speed}]`}}</div>
            </div>
            <div class="stream">
              <div class="info">{{s.info}}</div>
              <button v-if="video_need_convert(f, s)" @click.stop.prevent="to_x264(f, s)" class="ff">转H264+MP3</button>
              <button v-else-if="s.type === 'Audio'" @click.stop.prevent="extract_mp3(f, s)" class="ff">提取mp3</button>
              <button v-else-if="s.type === 'Subtitle'" @click.stop.prevent="extract_vtt(f, s)" class="ff">提取vtt</button>
            </div>       
          </div>
        </div>
      </div>
    </div>
    <div class="pad-bottom"></div>
    <div class="video-player" @click.prevent.stop="window.$('#video-player').toggle()"> 
      <div class="drag-header">
        <div class="prompt">
          <div>{{$t('current-playback')}}: {{cur_video?cur_video.name:$t('none')}} </div>
          <div><b>{{$t('drag-it-move')}}</b></div>
        </div>
        <p>{{`${$t('tip')}${$t('play-tip')}`}}</p>
      </div>
      <div>
        <video id="video-player" v-if="cur_video" :src="cur_url" controls autoplay preload="metadata">
          <track v-for="(vtt,i) in vtt_url" :key="i" :label="vtt.lang" kind="subtitles" :src="vtt.url" :default="0==i">
        </video>
      </div>
    </div>
  </div>
</template>

<script>
import _ from 'lodash'
import { mapGetters, mapActions } from 'vuex'
import util from "@/common/util";

export default {
  name: "video",
  created: function() {
    this.$root.$on("ffmpeg_progress", this.ffmpeg_progress);
  },
  destroyed() {
    this.$root.$off("ffmpeg_progress", this.ffmpeg_progress);
  },
  mounted() {
    this.draggie = new Draggabilly('.video-player', {
      containment: '.video', 
      handle: '.drag-header'
    });
    this.load_metadata()
  },
  data() {
    return {
      play_type: 1,
      cur_video: null,
      durations:{},
      streams: {}
    };
  },
  watch: {
    'files' (new_value, old_value) {
      this.streams = {}
      this.durations = {}
    }
  },
  computed: {
    ...mapGetters({
      all: 'all',
      files: 'videos',
    }),
    ...mapGetters([
      'file_path',
      'file_url'
    ]),
    cur_url() {
      return this.cur_video ? this.file_url(this.cur_video.name) : null;
    },
    vtt_url() {
      if(this.cur_video){
        const vp = this.cur_video.name;
        let vtt_name_to_find;
        if(vp.substr(vp.length - 9) === '_x264.mp4'){
          vtt_name_to_find = vp.substr(0, vp.length - 9);
        } else{
          vtt_name_to_find = vp.substr(0, vp.length - 4);
        }
        return this.all.filter(f=> 
          f.name.substr(f.name.length - 4) == ".vtt" 
          && f.name.includes(vtt_name_to_find)
        ).map( f=>({lang: this.get_lang_from_path(f.name, vtt_name_to_find), url: this.file_url(f.name)}) );
      }
    }
  },
  methods: {
    ffmpeg_progress(noty){
      // console.log(JSON.stringify(noty));
      const vs = this.streams[noty.video];
      if(vs){
        for (const s of vs) {
          if(s.sid == noty.sid){
            s.progress = noty.progress;
            s.speed = noty.speed;
            // console.log(`s.progress=${s.progress}, s.speed=${s.speed}`)
            vs.splice(vs.indexOf(s), 1, s);
            break;
          }
        }
      }
    },
    video_need_convert(f, s){
      if('Video' === s.type){
          // let is_yuv420p = str.search(/yuv420p,/);
          let is_yuv420p = /yuv420p,/.test(s.info);
          return s.codec != 'h264' || !is_yuv420p || f.name.substr(f.name.length - 4) != ".mp4";
      }
      return false;
    },
    async extract_vtt(f, s){
      try {
        const res = await util.post_local("ffmpeg", { code: 2, video: f.path, sid: s.sid, lang: s.lang }, 1);
        if(res.ret == 0){          
          
        } else{

        }
      } catch (err) {
        // console.log(`${f.name}转vtt failed: ${JSON.stringify(err)}`);
      }
    },
    async extract_mp3(f, s){
      try {
        const res = await util.post_local("ffmpeg", { code: 3, video: f.path, sid: s.sid, lang: s.lang }, 1);
        if(res.ret == 0){          
          
        } else{

        }
      } catch (err) {
        // console.log(`${f.name}转vtt failed: ${JSON.stringify(err)}`);
      }
    },
    async to_x264(f, s){
      try {
        const res = await util.post_local("ffmpeg", { code: 4, video: f.path }, 1);
        if(res.ret == 0){          
          
        } else{

        }
      } catch (err) {
        // console.log(`${f.name}转vtt failed: ${JSON.stringify(err)}`);
      }
    },
    async get_info(f, e){
      // console.log(`in get_info, f.path=${f.path}`)
      const cur_minfo = $(e.currentTarget).closest('.file-desc').find('.media-info');
      if(this.streams[f.path]) {
        cur_minfo.toggle();
        return;
      }
      try {
        const res = await util.post_local("ffmpeg", { code: 0, media: f.path }, 1);
        if(res.ret == 0){          
          this.streams = { ...this.streams, [f.path]: res.streams.trim().split(/\n/).map(si=>{
            console.log(si);
            const m = si.match(/#(0:\d+)(?:\((\w+)\))?:\s(\w+):\s(\w+)/);
            return {
              info: si,
              sid: m[1],
              lang: m[2] || 'und',
              type: m[3],
              codec: m[4]
            };
// console.log(m[0]);  //#0:3(chi): Subtitle: mov_text or #0:0(und): Video: h264
// console.log(m[1])   //0:3
// console.log(m[2])   //chi or und
// console.log(m[3])   //Subtitle or Video or Audio
// console.log(m[4])   //mov_text or h264 or aac
          })};
          cur_minfo.show();
          // util.show_alert_top_tm(`[${f.name}]转vtt成功`);
        } else{

        }
      } catch (err) {
        // console.log(`${f.name}转vtt failed: ${JSON.stringify(err)}`);

      }
    },
    get_lang_from_path(p, strip){
      let t = p.substr(strip.length)
      let lang = 'und'
      t = t.substr(0, t.length -4)
      // console.log(`get_lang_from_path, t=${t}`)
      if(t){
          t = util.match_unicode(t);
          // console.log(`t=${t}`)
          if(t){
              lang = t[0];
          }
      }
      return lang;
    },
    formatFileSize(bytes, decimalPoint) {
      return util.formatFileSize(bytes, decimalPoint)
    },
    load_metadata(){
      this.files.forEach(f=>{
        let video = document.createElement("video");
        $(video).on("loadedmetadata", ()=>{
          this.durations = {...this.durations, [f.name]: util.toHHMMSS(video.duration)};
        });
        video.src = this.file_url(f.name);
      })
    },
    play_video(f, e) {
      this.cur_video = f;
      $('.fi').removeClass('selected')
      $(e.currentTarget).addClass('selected')
    },

  }
};
</script>
<style scoped >
.media-info{
  display: none;
}
.info{
  flex: 1;
  border: inset;
}
.stream{
  display: flex;
  align-items: center;
  border: .1em outset;
  background-color: azure;
}
.video{
  min-height: 97%;
  position: relative;
}
video{
  width: 100%;
}
.drag-header{
  padding: 0 1em;
  color: white;
  background-color: black;
  text-align: left;
  font-size: 0.8rem;
  opacity: 0.79;
}
.prompt{
  display: flex;
  justify-content: space-between;
  text-align: center;
}
b{
  color: red;
}
.play-type{
  display: flex;
  align-items: center;
  background-color: rgb(190, 190, 190);
}
button{
  font-size: 0.9rem;
  border-radius: .7em;
  padding: 0.3em;
}

.file-desc {
  flex: 1;
  margin: 0 0.4em;
}

.file-time {
  display: flex;
  align-items: center;
  justify-content: space-between;
  font-size: 0.7rem;
}

.fi {
  display: flex;
  align-items: center;
  text-align: left;
  background-color: rgb(177, 250, 250);
  margin: 0.3em 0;
  border: 2px inset black;
}
button:disabled {
  /* background-color: #ccc; */
  color: grey;
}
input[type="radio"]{
  height: 1.3em;
  width: 1.3em;
}
.selected{
  font-size: 1.1rem;
  background-color: aqua;
}
.progressbar {
  position: relative;
  margin: 0.3em 0.5em;
  /* min-height: 1.3rem; */
  background-color: black;
  border-radius: 0.7em; /* (height of inner div) / 2 + padding */
  padding: 0.2em;
  overflow: hidden;
}
.progressbar > .cap {
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translateX(-50%) translateY(-50%);
  background-color: initial;
  width: 100%;
  color: rgb(250, 163, 65);
  font-weight: bold;
}

.progressbar > div {
  text-align: center;
  background-color: rgb(87, 167, 156);
  height: 1.5em;
  line-height: 1.5em;
  border-radius: 0.7em;
}
</style>
