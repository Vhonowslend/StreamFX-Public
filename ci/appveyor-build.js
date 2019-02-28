const cp = require('child_process');

var config32 = cp.spawn(
	"cmake", [
		'-H.',
		'-B"build/32"',
		'-G"Visual Studio 15 2017"',
		'-DCMAKE_INSTALL_PREFIX="build/distrib"',
		'-DCMAKE_PACKAGE_PREFIX="build"',
		'-DCMAKE_PACKAGE_NAME="obs-stream-effects"'
	], {
		windowsVerbatimArguments: true,
		windowsHide: true
	}
);
config32.stdout.on('data', (data) => {
	process.stdout.write(`[32:Out] ${data}`);
});
config32.stderr.on('data', (data) => {
	console.log(`[32:Err] ${data}`);
});
config32.on('exit', (code, signal) => {
	if (code != 0) {
		process.exit(code)
	}
	var build32 = cp.spawn(
		"cmake", [
			'--build build/32',
			'--target INSTALL',
			'--config RelWithDebInfo',
			'--',
			'/logger:"C:\\Program Files\\AppVeyor\\BuildAgent\\Appveyor.MSBuildLogger.dll"'
		], {
			windowsVerbatimArguments: true,
			windowsHide: true
		}
	);
	build32.stdout.on('data', (data) => {
		process.stdout.write(`[32:Out] ${data}`);
	});
	build32.stderr.on('data', (data) => {
		process.stderr.write(`[32:Err] ${data}`);
	});
	build32.on('exit', (code, signal) => {
		if (code != 0) {
			process.exit(code)
		}
	});
});

var config64 = cp.spawn(
	"cmake", [
		'-H.',
		'-B"build/64"',
		'-G"Visual Studio 15 2017 Win64"',
		'-DCMAKE_INSTALL_PREFIX="build/distrib"',
		'-DCMAKE_PACKAGE_PREFIX="build"',
		'-DCMAKE_PACKAGE_NAME="obs-stream-effects"'
	], {
		windowsVerbatimArguments: true,
		windowsHide: true
	}
);
config64.stdout.on('data', (data) => {
	process.stdout.write(`[64:Out] ${data}`);
});
config64.stderr.on('data', (data) => {
	console.log(`[64:Err] ${data}`);
});
config64.on('exit', (code, signal) => {
	if (code != 0) {
		process.exit(code)
	}
	var build64 = cp.spawn(
		"cmake", [
			'--build build/64',
			'--target INSTALL',
			'--config RelWithDebInfo',
			'--',
			'/logger:"C:\\Program Files\\AppVeyor\\BuildAgent\\Appveyor.MSBuildLogger.dll"'
		], {
			windowsVerbatimArguments: true,
			windowsHide: true
		}
	);
	build64.stdout.on('data', (data) => {
		process.stdout.write(`[32:Out] ${data}`);
	});
	build64.stderr.on('data', (data) => {
		process.stderr.write(`[32:Err] ${data}`);
	});
	build64.on('exit', (code, signal) => {
		if (code != 0) {
			process.exit(code)
		}
	});
});

