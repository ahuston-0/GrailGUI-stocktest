#include "audio/AudioPlayer.hh"

using namespace std;
AudioPlayer::AudioPlayer() { newContext("default"); }

AudioPlayer::~AudioPlayer() {
  for (auto pair = contexts.begin(); pair != contexts.end(); pair++) {
    mpv_terminate_destroy(pair->second);
  }

  // c++ 17 feature
  // for (const auto &[name, ctx] : contexts) {
  //   mpv_terminate_destroy(ctx);
  // }
}

void AudioPlayer::newContext(string name) {
  mpv_handle *ctx = mpv_create();

  if (!ctx) {
    printf("failed creating mpv context\n");
    throw Ex1(Errcode::MPV_FAILURE);
  }

  checkError(mpv_initialize(ctx));
  checkError(mpv_command_string(ctx, "cycle pause"));
  const char *cmd[] = {"set", "video", "no", nullptr};
  checkError(mpv_command(ctx, cmd));
  contexts[name] = ctx;
}

void AudioPlayer::setCurrentContext(string name) {
  auto pair = contexts.find(name);
  if (pair != contexts.end()) {
    currentCtx = pair->second;
  } else {
    printf("Couldn't set a context with name: %s\n", name.c_str());
  }
}

void AudioPlayer::addFile(string filePath) {
  const char *cmd[] = {"loadfile", filePath.c_str(), "append", nullptr};
  checkError(mpv_command(currentCtx, cmd));
}

void AudioPlayer::addPlaylist(string filePath, bool append) {
  const char *cmd[] = {"loadlist", filePath.c_str(),
                       (append) ? "append" : "replace", nullptr};
  checkError(mpv_command(currentCtx, cmd));
}

void AudioPlayer::setVolume(int volume) {
  if (volume < 1000 && volume >= 0) {
    char cmd[25];
    sprintf(cmd, "set volume %d", volume);
    checkError(mpv_command_string(currentCtx, cmd));
  } else {
    printf("Please provide a volume that is between 0 and 999\n");
  }
}

void AudioPlayer::seekLocation(string time, string type) {
  if (type == "relative" || type == "absolute" || type == "relative-percent" ||
      type == "absolute-percent") {
    const char *cmd[] = {"seek", time.c_str(), type.c_str(), nullptr};
    checkError(mpv_command(currentCtx, cmd));
  } else {
    printf(
        "Please provide one of the following for seek type: \nrelative, "
        "absolute, relative-percent, absolute-percent\n");
  }
}

void AudioPlayer::revertSeek() {
  const char *cmd[] = {"revert-seek", nullptr};
  checkError(mpv_command(currentCtx, cmd));
}

void AudioPlayer::playlistNext() {
  const char *cmd[] = {"playlist-next", nullptr};
  checkError(mpv_command(currentCtx, cmd));
}

void AudioPlayer::playlistPlayIndex(int index) {
  char indexString[67];
  sprintf(indexString, "%d", index);
  const char *cmd[] = {"set", "playlist-pos", indexString, nullptr};
  checkError(mpv_command(currentCtx, cmd));
}

void AudioPlayer::playlistPrev() {
  const char *cmd[] = {"playlist-prev", nullptr};
  checkError(mpv_command(currentCtx, cmd));
}

void AudioPlayer::playlistClear() {
  const char *cmd[] = {"playlist-clear", nullptr};
  checkError(mpv_command(currentCtx, cmd));
}

void AudioPlayer::playlistRemoveIndex(int index) {
  char indexString[67];
  sprintf(indexString, "%d", index);
  const char *cmd[] = {"playlist-remove", indexString, nullptr};
  checkError(mpv_command(currentCtx, cmd));
}

void AudioPlayer::playlistMove(int index1, int index2) {
  char index1String[67];
  char index2String[67];
  sprintf(index1String, "%d", index1);
  sprintf(index2String, "%d", index2);
  const char *cmd[] = {"playlist-move", index1String, index2String, nullptr};
  checkError(mpv_command(currentCtx, cmd));
}

void AudioPlayer::playlistShuffle() {
  const char *cmd[] = {"playlist-shuffle", nullptr};
  checkError(mpv_command(currentCtx, cmd));
}

void AudioPlayer::togglePause() {
  checkError(mpv_command_string(currentCtx, "cycle pause"));
  isPlaying = !isPlaying;
}

void AudioPlayer::setPlaying() {
  if (!isPlaying) {
    checkError(mpv_command_string(currentCtx, "cycle pause"));
    isPlaying = !isPlaying;
  }
}

void AudioPlayer::setPaused() {
  if (isPlaying) {
    checkError(mpv_command_string(currentCtx, "cycle pause"));
    isPlaying = !isPlaying;
  }
}

void AudioPlayer::printCurrentTime() {
  mpv_node result;

  checkError(
      mpv_get_property(currentCtx, "time-pos", MPV_FORMAT_NODE, &result));
  printf("Track Time Elapsed in Seconds: %.2f\n", result.u.double_);
  mpv_free_node_contents(&result);
}

// as of now get_property on playlist/count returns one, and getting the
// filename at that entry gives the filepath to the playlist.txt file, or the
// youtube playlist link, instead of the components of these things
#if 0
void AudioPlayer::playlistPrintEntries() {
  mpv_node result;
  checkError(
      mpv_get_property(currentCtx, "playlist/count", MPV_FORMAT_NODE, &result));
  int numEntries = result.u.flag;
  printf("num entries: %d", numEntries);
  string entryNames[numEntries];
  char property[85];

  for (int i = 0; i < numEntries; i++) {
    sprintf(property, "playlist/%d/filename", i);
    checkError(
        mpv_get_property(currentCtx, property, MPV_FORMAT_NODE, &result));
    entryNames[i] = result.u.string;
  }

  for (auto &name : entryNames) {
    printf("%s\n", name.c_str());
  }
}
#endif
