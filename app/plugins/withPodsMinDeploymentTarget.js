// Xcode 27 fails an archive on any pod target below iOS 15.0:
//   "IPHONEOS_DEPLOYMENT_TARGET is set to 12.4, but the range of supported
//    deployment target versions is 15.0 to 27.0.x (target RNSVG-RNSVGFilters)"
// react-native-svg's podspec still says 12.4 and CocoaPods hands resource
// bundle targets the spec's platform, not the Podfile's. Raise every pod
// target to the app minimum in post_install. `expo prebuild --clean` rewrites
// the Podfile, so this lives in a config plugin instead of a hand edit.
const { withDangerousMod } = require('expo/config-plugins');
const fs = require('fs');
const path = require('path');

// ponytail: 15.1 is the Expo Podfile default; read it from the Podfile if it ever moves.
const MIN = '15.1';
const HOOK = `
    # Raise pods below the app minimum (plugins/withPodsMinDeploymentTarget.js).
    installer.pods_project.targets.each do |target|
      target.build_configurations.each do |config|
        if config.build_settings['IPHONEOS_DEPLOYMENT_TARGET'].to_f < ${MIN}
          config.build_settings['IPHONEOS_DEPLOYMENT_TARGET'] = '${MIN}'
        end
      end
    end
`;

module.exports = (config) =>
  withDangerousMod(config, [
    'ios',
    (cfg) => {
      const podfile = path.join(cfg.modRequest.platformProjectRoot, 'Podfile');
      const src = fs.readFileSync(podfile, 'utf8');
      if (!src.includes('withPodsMinDeploymentTarget')) {
        const out = src.replace(/post_install do \|installer\|\n/, (m) => m + HOOK);
        if (out === src) throw new Error('withPodsMinDeploymentTarget: no post_install block in Podfile');
        fs.writeFileSync(podfile, out);
      }
      return cfg;
    },
  ]);
