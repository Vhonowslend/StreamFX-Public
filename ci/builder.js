"use strict";

const process = require('process');
const runner = require('./runner.js');
let env = process.env;

let x32_steps = [];
let x64_steps = [];

if ((process.platform == "win32") || (process.platform == "win64")) {
	// Windows
	let extra_conf = [
		`-DCMAKE_SYSTEM_VERSION=${process.env.CMAKE_SYSTEM_VERSION}`,
		`-DCMAKE_PACKAGE_NAME=obs-ffmpeg-encoder`,
		'-DCMAKE_INSTALL_PREFIX="build/distrib/"',
		'-DCMAKE_PACKAGE_PREFIX="build/package/"',
	];
	let extra_build = [

	];

	if(process.env.APPVEYOR) {
		extra_build.concat(['--', '/logger:"C:\\Program Files\\AppVeyor\\BuildAgent\\Appveyor.MSBuildLogger.dll"']);
	}

	if ((process.env.CMAKE_GENERATOR_32 !== undefined) && (process.env.CMAKE_GENERATOR_32 !== "")) {
		x32_steps.push(
			[ 'cmake', [
				'-H.', '-Bbuild/32',
				`-G"${process.env.CMAKE_GENERATOR_32}"`, '-AWin32', '-T"host=x64"',
			].concat(extra_conf), env ]
		);
		x32_steps.push(
			[ 'cmake', [
				'--build', 'build/32', 
				'--config', 'RelWithDebInfo',
				'--target', 'INSTALL'
			].concat(extra_build), env ]
		);
	}
	if ((process.env.CMAKE_GENERATOR_64 !== undefined) && (process.env.CMAKE_GENERATOR_64 !== "")) {
		x64_steps.push(
			[ 'cmake', [
				'-H.', '-Bbuild/64',
				`-G"${process.env.CMAKE_GENERATOR_64}"`, '-Ax64', '-T"host=x64"',
			].concat(extra_conf), env ]
		);
		x64_steps.push(
			[ 'cmake', [
				'--build', 'build/64', 
				'--config', 'RelWithDebInfo',
				'--target', 'INSTALL'
			].concat(extra_build), env ]
		);
	}
} else {
	// Unix

}

function runRunners(runnerArray, name) {
	return new Promise(async (resolve, reject) => {
		let local = runnerArray.reverse();
		while (local.length > 0) {	
			try {
				let task = local.pop();
				let work = new runner(name, task[0], task[1], task[2]);
				await work.run();
			} catch (e) {
				reject(e);
				return;
			}
		}
		resolve(0);
	});
}

let promises = [];
promises.push(runRunners(x32_steps, "32-Bit"));
promises.push(runRunners(x64_steps, "64-Bit"));
Promise.all(promises).then(
	res => {
		process.exit(0);
	},
	err => {
		console.log(err);
		process.exit(1);
	}
).catch(err => {
	console.log(err);
	process.exit(1);
})
