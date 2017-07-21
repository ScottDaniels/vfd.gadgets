/*
	Mnemonic:	jwrapper_test2
	Abstract: 	This will attempt to put the jwrapper code through its paces.

				Unlike jwapper_test.c which allows a user supplied set of json to be given
				this uses a static json string so as to test specific things and report
				a binary good/bad for each.

				Example:

	Author:		E. Scott Daniels
	Date:		31 March 2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gadgetlib.h"

char* raw_json = "{ \
	\"patient_id\": 1027844, \
	\"active_patient\": true,\
	\"last_visit\": \"2015/02/03\",\
	\"patient_info\": {\
		\"first_name\": \"Fred\",\
		\"last_name\": \"Flintsone\",\
		\"dob\": \"1963/04/03\",\
		\"sex\": \"M\",\
		\"weight_kilo\": 65,\
		\"drug_alergies\": [ \"asprin\",\"darvaset\" ]\
	}\
	\"Contact_info\": {\
		\"name\": \"Wilma\", \"relation\": \"wife\", \"phone\": \"972.612.8918\"\
	}\
	\"family\": [\
		{ \"relative\": \"wife\", \"age\": 40, \"name\": \"Wilma\", \"blood\": false, },\
		{ \"relative\": \"daugher\", \"age\": 10, \"name\": \"Pebbles\", \"blood\": true, },\
		{ \"relative\": \"mother\", \"age\": 70, \"name\": \"Gertrude\", \"blood\": true, }\
	],\
	\"last_blood\": [ \"acceptable\", 192.1, 45.0, true, false, null, \"full pannel\" ]\
}";


/*
	Look up the string and return error count if not found.
*/
static int check_str( void* jblob, char* field, char* expect ) {
	char* stuff;
	
	if(  (stuff = jw_string( jblob, field )) ) {
		fprintf( stderr, "found %s: (%s)\n", field, stuff );
		if( strcmp( stuff, expect ) != 0 ) {
			fprintf( stderr, "error: %s string didn't match expected value (%s != %s)\n", field, stuff, expect );
			return 1;
		}
	} else {
		fprintf( stderr, "error: did not find %s string\n", field );
		return 1;
	}

	return 0;
}

/*
	Check element types in the "last_blood" array.
*/
static int check_ele_types( void* jblob ) {
	int ec = 0;

	if( !jw_is_value_ele( jblob, "last_blood", 1 )  ) {
		fprintf( stderr, "error: 'last_blood' array element 1 is not reporting type value\n" );
		ec++;
	}
	if( jw_is_value_ele( jblob, "last_blood", 0 )  ) {
		fprintf( stderr, "error: 'last_blood' array element 0 is reporting type value and should not be\n" );
		ec++;
	}
	if( !jw_is_bool_ele( jblob, "last_blood", 3 )  ) {
		fprintf( stderr, "error: 'last_blood' array element 1 is not reporting type boolean\n" );
		ec++;
	}
	if( !jw_is_bool_ele( jblob, "last_blood", 4 )  ) {
		fprintf( stderr, "error: 'last_blood' array element 3 is not reporting type boolean\n" );
		ec++;
	}
	if( jw_is_bool_ele( jblob, "last_blood", 2 )  ) {
		fprintf( stderr, "error: 'last_blood' array element 2 is reporting type boolean and should not be\n" );
		ec++;
	}
	if( jw_is_bool_ele( jblob, "last_blood", 0 )  ) {
		fprintf( stderr, "error: 'last_blood' array element 0 is reporting type boolean and should not be\n" );
		ec++;
	}

	if( ! ec ) {
		fprintf( stderr, "ok: all element primative type checks pass\n" );
	}

	return ec;
}

/*
	Check the type in the blob for boolean. Expect is 1 if we expect it to 
	be bool, and 0 if we expect it to not be bool. Returns the error count.
*/
static int check_type_bool( void* jblob, char* field, float expect ) {
	float	value;
	int		ec = 0;
	int		is_bool;

	is_bool = jw_is_bool( jblob, field );
	if( is_bool && expect == 1 ) {
		fprintf( stderr, "ok: %s reports boolean as expected\n", field );
		return 0;
	}

	if( is_bool ) {
		fprintf( stderr, "error: %s reports boolean but isn't expected to be\n", field );
		return 1;
	}

	if( expect == 1 ) {
		fprintf( stderr, "error: %s reports NOT boolean but is expected to be\n", field );
		return 1;
	}

	fprintf( stderr, "ok: %s reports NOT boolean as expected\n", field );
	return 0;			// not expected to be and didn't report boolean
}

/*
	Check the type in the blob for value. Expect is 1 if we expect it to 
	be a value, and 0 if we expect it to not be a value. Returns the error count.
*/
static int check_type_value( void* jblob, char* field, float expect ) {
	float	value;
	int		ec = 0;
	int		is_value;

	is_value = jw_is_value( jblob, field );
	if( is_value && expect == 1 ) {
		fprintf( stderr, "ok: %s reports it is a value as expected\n", field );
		return 0;
	}

	if( is_value ) {
		fprintf( stderr, "error: %s reports it is a value but isn't expected to be\n", field );
		return 1;
	}

	if( expect == 1 ) {
		fprintf( stderr, "error: %s reports it is NOT a value but is expected to be\n", field );
		return 1;
	}

	fprintf( stderr, "ok: %s reports it is NOT a value as expected\n", field );
	return 0;			// not expected to be and didn't report as a value
}

static int check_value( void* jblob, char* field, float expect ) {
	float	value;

	if(  (value = jw_value( jblob, field )) ) {
		fprintf( stderr, "found %s:  %0.2f\n", field, value );
		if( value != expect ) {
			fprintf( stderr, "error: %s value did not match the expected value: %.2f != %.2f\n", field, expect, value );
			return 1;
		}
	} else {
		fprintf( stderr, "error: did not find %s value\n", field );
		return 1;
	}

	return 0;
}


int main( int argc, char **argv ) {
	void*	jblob;						// parsed json stuff
	void*	sub_blob;					// nested object
	char	*stuff;
	float	value;
	int		errors = 0;

	if( (jblob = jw_new( raw_json )) == NULL ) {
		fprintf( stderr, "failed to create wrapper\n" );
		exit( 1 );
	}
	
	fprintf( stderr, "\ntesting outer layer things\n" );
	errors += check_str( jblob, "last_visit", "2015/02/03" );
	errors += check_value( jblob, "patient_id", 1027844.0 );
	errors += check_value( jblob, "active_patient", 1.0 );

	fprintf( stderr, "\ntesting array and embedded object in an array\n" );
	if( (value = jw_array_len( jblob, "family" )) == 3 ) {				// dig into the array and get the blob
		fprintf( stderr, "family array found and has 3 elementes as expected\n" );
		if( (sub_blob = jw_obj_ele( jblob, "family", 1 )) != NULL ) {
			errors += check_str( sub_blob, "name", "Pebbles" );
			errors += check_value( sub_blob, "blood", 1.0 );
			errors += check_value( sub_blob, "age", 10 );
		} else {
			fprintf( stderr, "error: family array has expected number of elements, but did not return element 1\n" );
			errors++;
		}
	} else {
		fprintf( stderr, "error: wrong number of elements for family arry: expected 3 got %.2f\n", value );
		errors++;
	}

	fprintf( stderr, "\ntesting embedded object at the outer level\n" );
	if( (sub_blob = jw_blob( jblob, "Contact_info" )) != NULL ) {			// should be able to reach it by loading the blob and then referencing into it
		fprintf( stderr, "found embedded object Contact_info\n" );
		errors += check_str( sub_blob, "relation", "wife" );
		errors += check_str( sub_blob, "phone", "972.612.8918" );
	} else {
		fprintf( stderr, "error: didn't get a direct reference to contact_info blob\n" );
		errors++;
	}

	errors += check_str( jblob, "Contact_info.relation", "wife" );		// should be able to reach it through dotted notation too

	// check to see if the primative types are reporting correctly
	fprintf( stderr, "\nchecking that primative types report correctly\n" );
	errors += check_type_bool( jblob, "last_visit", 0 );				// shouldn't report boolean
	errors += check_type_bool( jblob, "patient_id", 0 );				// shouldn't report boolean
	errors += check_type_bool( jblob, "active_patient", 1 );			// should report boolean

	errors += check_type_value( jblob, "last_visit", 0 );				// shouldn't report value
	errors += check_type_value( jblob, "active_patient", 0 );			// shouldn't report value
	errors += check_type_value( jblob, "patient_id", 1 );				// should report value

	errors += check_ele_types( jblob );


	jw_nuke( jblob );


	// ----------------------------------------------------------------------------------------
	if( errors ) {
		fprintf( stderr, "%d errors found  [FAIL]\n", errors );
	} else {
		fprintf( stderr, "[PASS]\n" );
	}

	return 0;
}
