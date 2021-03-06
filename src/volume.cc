/* Generates volumbe bar from Master mixer.
 *
 * Copyright 2014 Emil Edholm <emil@edholm.it>
 */
#include <sys/soundcard.h>
#include <errno.h>
#include "volume.hh"
#include "common.hh"

using namespace std;

void Volume::init_mixer() {
    int err;
    if ((err = snd_mixer_open(&h_mixer, 1)) < 0) {
        error_close("Mixer open error: %s\n", err, NULL);
        return;
    }

    if ((err = snd_mixer_attach(h_mixer, ATTACH)) < 0){
            error_close("Mixer attach error: %s\n", err, h_mixer);
        return;
    }

    if ((err = snd_mixer_selem_register(h_mixer, NULL, NULL)) < 0){
            error_close("Mixer simple element register error: %s\n", err,
                        h_mixer);
        return;
    }

    if ((err = snd_mixer_load(h_mixer)) < 0) {
            error_close("Mixer load error: %s\n", err, h_mixer);
        return;
    }

    snd_mixer_selem_id_alloca(&sid);
    if(sid) {
        snd_mixer_selem_id_set_index(sid, 0);
        snd_mixer_selem_id_set_name(sid, SELEM_NAME);
    } else {
        error_close("Mixer ID is null", 0, h_mixer);
        return;
    }

    if ((elem = snd_mixer_find_selem(h_mixer, sid)) == NULL) {
            error_close("Cannot find simple element\n", 0, h_mixer);
        return;
    }

    setup_failed = false;
}

void Volume::error_close(const char *errmsg, int err, snd_mixer_t *h_mixer) {
    if (err == 0)
        fprintf(stderr, errmsg);
    else
        fprintf(stderr, errmsg, snd_strerror(err));
    if (h_mixer != NULL)
        snd_mixer_close(h_mixer);
    setup_failed = true;
}

bool Volume::is_muted() {
    if(setup_failed) { return true; };

    int pb_switch;
    snd_mixer_selem_get_playback_switch(elem, CHANNEL, &pb_switch);
    return (pb_switch != 1);
}

int Volume::get_volume() {
    if(setup_failed) { return 0; };
    long vol;
    long vol_min, vol_max;

    snd_mixer_selem_get_playback_volume(elem, CHANNEL, &vol);
    snd_mixer_selem_get_playback_volume_range(elem, &vol_min, &vol_max);

    return 100 * vol / vol_max;
};

string Volume::generate_json() {
    init_mixer();
    map<string, string> m;
    if(setup_failed) {
         m["full_text"] = "<Mixer init failed>";
    } else {
        int volume = get_volume();
        if(is_muted() || volume == 0) {
            m["full_text"] = "  \uf026  ";
            m["color"] = "red";
        } else if(volume > 50) {
            m["full_text"] = "  \uf028  ";
        } else {
            m["full_text"] = "  \uf027  ";
        }
        snd_mixer_close(h_mixer);
    }
    return Common::map_to_json(m);
};
