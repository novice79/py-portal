<template>
  <div class="image">

    <div class="fi" v-for="f in files" >
      <div class="file-desc">
        <div>{{f.name}}</div>
        <div class="file-time">
          <div>{{f.time}}</div>
          <div>{{formatFileSize(f.size)}}</div>
        </div>
        <img @click="toggle_size" :src="file_url(f.path)" />
      </div>
    </div>
    <div class="pad-bottom"></div>
  </div>
</template>

<script>
import _ from 'lodash'
import util from "@/common/util";

export default {
  name: "image",
  created: function() {
    this.$root.$on("update_file_list", this.update_file_list);
  },
  destroyed() {
    this.$root.$off("update_file_list", this.update_file_list);
  },
  mounted() {
    this.files = this.filter_img(g.files);
  },
  data() {
    return {
      files: []
    };
  },
  computed: {
    store_url() {
      return '';
    }
  },
  methods: {
    file_url(file_path) {
      return util.path2url(file_path);
    },
    formatFileSize(bytes, decimalPoint) {
      return util.formatFileSize(bytes, decimalPoint)
    },
    filter_img(files){
      return _.filter(files, f=>f.type && f.type.includes('image/'));
    },
    toggle_size(e) {
        $(e.target).toggleClass('ori-size')
    },
    update_file_list(files) {
      this.files = this.filter_img(files);
    },
  }
};
</script>
<style scoped >
.image{
  min-height: 97%;
  position: relative;
}

button{
  font-size: 1.5rem;
  border-radius: 1em;
  padding: 0.3em;
}

.file-desc {
  flex: 1;
  margin: 0 0.4em;
}

.file-time {
  display: flex;
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
img {
  width: 100%;
}
.ori-size{
  width: auto;
}
</style>
