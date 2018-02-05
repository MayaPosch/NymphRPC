
#pragma once
#ifndef STRUCTS_H
#define STRUCTS_H

#include "../../src/nymph.h"

#include "target_code.h"

// --- Global struct data.

// Create struct with 34 items in it.
NymphArray* createStudentsStruct() {
	NymphArray* studentsStruct = new NymphArray();

	for (uint8_t i = 0; i < 34; ++i) {
		NymphArray* student = new NymphArray();
		
		student->addValue(new NymphString(rand_str(43))); 	// name.
		student->addValue(new NymphUint32(rand() % 600));	// friends
		
		NymphArray* home_address = new NymphArray();
		home_address->addValue(new NymphString(rand_str(12)));			// city
		home_address->addValue(new NymphUint32(rand() % 3000 + 5000));	// zipcode
		home_address->addValue(new NymphString(rand_str(16)));			// street
		home_address->addValue(new NymphUint16(rand() % 125 + 1));		// number
		student->addValue(home_address);					// home_address
		
		NymphArray* birth_place = new NymphArray();
		birth_place->addValue(new NymphString(rand_str(12)));			// city
		birth_place->addValue(new NymphUint32(rand() % 3000 + 5000));	// zipcode
		birth_place->addValue(new NymphString(rand_str(16)));			// street
		birth_place->addValue(new NymphUint16(rand() % 125 + 1));		// number
		student->addValue(birth_place);						// birth_place
		
		NymphArray* birth = new NymphArray();
		birth->addValue(new NymphUint32(rand() % 30 + 1900));	// year
		birth->addValue(new NymphUint8(rand() % 12 + 1));		// month
		birth->addValue(new NymphUint8(rand() % 30 + 1));		// day
		student->addValue(birth);							// birth date
		
		NymphArray* favorite_subjects = new NymphArray();
		NymphArray* email_addresses = new NymphArray();
		NymphArray* schools = new NymphArray();
		for (uint8_t j = 0; j < 34; ++j) {
			NymphArray* subject = new NymphArray();
			subject->addValue(new NymphUint32(rand() % 28345));	// id
			subject->addValue(new NymphString(rand_str(22)));	// title
			subject->addValue(new NymphString(rand_str(16)));	// code
			favorite_subjects->addValue(subject);
			
			email_addresses->addValue(new NymphString(rand_str(37)));
			
			NymphArray* school = new NymphArray();
			school->addValue(new NymphString(rand_str(36)));	// name
			
			NymphArray* school_address = new NymphArray();
			school_address->addValue(new NymphString(rand_str(12)));			// city
			school_address->addValue(new NymphUint32(rand() % 3000 + 5000));	// zipcode
			school_address->addValue(new NymphString(rand_str(16)));			// street
			school_address->addValue(new NymphUint16(rand() % 125 + 1));		// number
			school->addValue(school_address);
			
			NymphArray* founding = new NymphArray();
			founding->addValue(new NymphUint32(rand() % 30 + 1900));	// year
			founding->addValue(new NymphUint8(rand() % 12 + 1));		// month
			founding->addValue(new NymphUint8(rand() % 30 + 1));		// day
			school->addValue(founding);
			
			NymphArray* school_emails = new NymphArray();
			for (uint8_t k = 0; k < 34; ++k) {
				school_emails->addValue(new NymphString(rand_str(37)));
			}
			
			school->addValue(school_emails);
		}
		
		student->addValue(favorite_subjects);
		student->addValue(email_addresses);
		student->addValue(schools);
		
		studentsStruct->addValue(student);
	}
	
	return studentsStruct;
}

// ---

#endif