<template>
  <div class="folder">
    <div class="op-btn">
      <button @click="restore_before_move">{{$t('back')}}</button>
      <button @click="confirm_move">{{$t('move')}}</button>
    </div>
    <div class="op-btn-dummy"></div>
    <div class="fi" :key="f.name" v-for="f in files" @click="enter(f.name)">
      <div>
        <i class="small material-icons">folder</i>
      </div>
      <div class="file-desc">
        <div>{{f.name}}</div>
        <div class="file-time">
          <div>{{f.time}}</div>
        </div>
      </div>
    </div>
    <div class="pad-bottom"></div>
  </div>
</template>

<script>
import _ from 'lodash'
import { mapGetters, mapActions } from 'vuex'

export default {
  name: "folder",
  created: function() {

  },
  destroyed() {

  },
  mounted() {

  },
  data() {
    return {
      placeholder: []
    };
  },
  computed: {
    ...mapGetters({
      files: 'dirs',
    }),
    ...mapGetters([
      'file_path',
      'file_url'
    ])
  },
  methods: {
    ...mapActions([
      'create_dir',
      'enter',
      'back',
      'move_to',
      'confirm_move',
      'restore_before_move'
    ]),
  }
};
</script>
<style scoped >
.folder{
  min-height: 97%;
}

button{
  font-size: 1.5rem;
  border-radius: 1em;
  padding: 0.3em;
}
.op-btn-dummy{
  min-height: 3.1em;
}
.op-btn{
  position: absolute;
  /* top: -0.4em; */
  width: 100%;

  display: flex;
  justify-content: space-around;
  background-color: rgb(174, 209, 206);
  opacity: 0.9;
}
.file-desc {
  flex: 1;
  margin: 0 0.4em;
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
}
button:disabled {
  /* background-color: #ccc; */
  color: grey;
}

</style>
