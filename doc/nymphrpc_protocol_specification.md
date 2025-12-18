# NymphRPC Binary Protocol

This document defines the NymphRPC protocol. It's a binary protocol for remote procedure calls.

Details:
- Little Endian format: all binary messages are in LE byte format.
- All fields are 8 to 64 bytes long, in unsigned integer format.

---

## Protocol 

Each binary message starts with the same header, followed by a message-specific payload.

**Header**

<pre>
uint32		Signature: DRGN (0x4452474e)
uint32		Total message bytes following this field.
uint8		Protocol version (0x00).
uint32		Method ID: identifier of the remote function.
uint32		Flags (see _Flags_ section).
uint64		Message ID. Simple incrementing global counter.
</pre>


**Flags**

<pre>
0x00	Regular message.
0x01	Reply message.
0x02	Exception message.
0x04	Callback message.
</pre>


**Regular message**

<pre>
&lt;header&gt;
&lt;..&gt;	Serialised values.
uint8		Message end. None type (0x01). See 'Types' section.
</pre>


**Reply message**

<pre>
&lt;header&gt;
uint64		ReplyTo ID: message ID that this is in response to.
&lt;..&gt;		Serialised reply.
uint8		Message end. None type (0x01). See 'Types' section.
</pre>


**Exception message**

<pre>
&lt;header&gt;
uint64		ReplyTo ID: message ID that this is in response to.
uint32		Exception ID.
uint8		Message end. None type (0x01). See 'Types' section.
</pre>


**Callback message**

<pre>
&lt;header&gt;
uint8		String typecode. (0x10)
uint8-64	String length.
uint64		ReplyTo ID: message ID that this is in response to.
&lt;..&gt;		Callback name (String).
&lt;..&gt;		Serialised values.
uint8		Message end. None typecode (0x01). See 'Types' section.
</pre>

----

## Types

Types in NymphRPC are divided into internal and external types. The external types are the types used by an application, while the internal ones are used by NymphRPC itself.

**Internal typecodes**

- All internal typecodes are serialised as Uint8 values.
- Unsigned integers are defined as <i>Uint*</i>.
- Signed integers are defined as <i>Sint*</i>.
- Float is 32-bit floating point.
- Double is 64-bit floating point.
- See the <i>Complex types</i> section for details on specific types.

<pre>
Null			0x00
None			0x01
Boolean false	0x02
Boolean true	0x03
Uint8			0x04
Sint8			0x05
Uint16			0x06
Sint16			0x07
Uint32			0x08
Sint32			0x09
Uint64			0x0a
Sint64			0x0b
Float			0x0c
Double			0x0d
Array			0x0e
Empty string	0x0f
String			0x10
Struct			0x11
Void			0x12
</pre>


**External typecodes**

<pre>
Null	0
Array	1
Bool	2
Uint8	3
Sint8	4
Uint16	5
Sint16	6
Uint32	7
Sint32	8
Uint64	9
Sint64	10
Float	11
Double	12
String	13
Struct	14
Any		15
</pre>

----

## Complex types

**String**

Strings in NymphRPC are defined by a length and the characters that make up the string. Internally during serialisation/deserialisation the String is either empty or has a length of 1+. 

The 'empty string' type (0x0f) is an optimisation that removes the need to specify a length field.

For non-empty strings, the length field can be specified as a Uint8, Uint16, Uint32 or Uint64, also as an optimisation. E.g. a length of &lt;=0xFF would fit in a Uint8.

E.g.:

<pre>
uint8	Typecode (String: 0x10)
uint16	Length (&gt; 0xff, &lt;= 0xffff)
</pre>


<b>Struct</b>

Structs are simple key/value pairs. They feature the following structure:

<pre>
uint8	Typecode (Struct: 0x11)
&lt;key/value pairs&gt;
uint8	Typecode (None, 0x01)
</pre>


<b>Array</b>

Arrays are defined as a count of elements followed by the element values.

<pre>
uint8	Typecode (Array: 0x0e)
uint64	Number of elements
&lt;elements&gt;
uint8	Typecode (None, 0x01)
</pre>
