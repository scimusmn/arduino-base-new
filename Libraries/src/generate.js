const { opendirSync, readFileSync, writeFileSync } = require('fs');


function DetermineSourceFiles() {
	const sourceFiles = [];
	let dir = opendirSync('.');
	for (let dirent = dir.readSync(); dirent != null; dirent = dir.readSync()) {
		if (dirent.isFile() && /.*\.hpp$/.test(dirent.name)) {
			sourceFiles.push(dirent.name)
		}
	}
	sourceFiles.sort();
	return sourceFiles;
}


function LoadFile(filename) {
	return readFileSync(filename).toString();
}


function GetSection(source, section) {
	const string = `\\/\\*\\s@${section}\\s\\*\\/([\\s\\S]*?)\\/\\*\\s@[A-Z]+\\s\\*\\/`;
	const regex = new RegExp(string, 'm');
	const matches = source.match(regex);
	if (matches) {
		// return first group
		return matches[1];
	} else {
		return '';
	}
}


function GetIncludeSet(source) {
	const section = GetSection(source, 'INCLUDE');
	const lines = section.split('\n');
	return new Set(lines.filter(line => /\S+/.test(line)));
}


function BuildFile(sourceFiles) {
	const includeSet = new Set();
	let headers = [];
	let externs = [];
	let implementations = [];
	let setups = [];

	for (let filename of sourceFiles) {
		const source = LoadFile(filename);
		GetIncludeSet(source).forEach(include => includeSet.add(include));
		headers.push(GetSection(source, 'HEADER'));
		externs.push(GetSection(source, 'EXTERN'));
		implementations.push(GetSection(source, 'IMPLEMENTATION'));
		setups.push(GetSection(source, 'SETUP'));
	}

	// create array of includes
	const includes = [];
	includeSet.forEach(i => includes.push(i));
	includes.sort();

	// clean header, extern, and implementation arrays
	headers = headers.filter(h => /\S+/.test(h));
	externs = externs.filter(e => /\S+/.test(e));
	implementations = implementations.filter(i => /\S+/.test(i));
	setups = setups.filter(s => /\S+/.test(s));

	const data = `/** @file smm.h */
#pragma once

/*******************************************************************************
 *
 * smm.h
 *
 * This is an automatically generated file - please do not edit it directly!
 *
 *******************************************************************************
 */

${includes.join('\n')}

namespace smm {

  void setup();

${headers.join('\n\n')}

/* end namespace smm */
}

/* externs */
${externs.join('\n')}

/*#############################################################################*
 #                                                                             #
 #                               IMPLEMENTATION                                #
 #                                                                             #
 *#############################################################################*/
#if defined(SMM_IMPLEMENTATION)
${implementations.join('\n\n')}

void smm::setup() {
${setups.join('\n')}
}
#endif
`;
	writeFileSync('../smm.h', data);
}


const sources = DetermineSourceFiles();
BuildFile(sources);
