#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>

int play_bgm_loop(const char *path, float volume) {
    @autoreleasepool {
        NSString *filePath = [NSString stringWithUTF8String:path];
        if (filePath == nil) {
            return 1;
        }

        NSURL *url = [NSURL fileURLWithPath:filePath];
        NSError *error = nil;
        AVAudioPlayer *player = [[AVAudioPlayer alloc] initWithContentsOfURL:url error:&error];
        if (player == nil) {
            return 1;
        }

        player.volume = volume;
        player.numberOfLoops = -1;

        if (![player prepareToPlay] || ![player play]) {
            return 1;
        }

        [[NSRunLoop currentRunLoop] run];
    }

    return 0;
}
